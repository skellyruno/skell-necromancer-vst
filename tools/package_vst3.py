#!/usr/bin/env python3
"""Create a verified, provenance-bearing Windows x64 VST3 ZIP package."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import struct
import sys
import zipfile
from pathlib import Path


class PackageError(RuntimeError):
    """Raised when a build tree is not a safe distributable VST3 bundle."""


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _read_pe_machine(binary: Path) -> int:
    with binary.open("rb") as stream:
        header = stream.read(64)
        if len(header) < 64 or header[:2] != b"MZ":
            raise PackageError(f"VST3 binary is not a PE executable: {binary}")
        pe_offset = struct.unpack_from("<I", header, 0x3C)[0]
        if pe_offset < 64:
            raise PackageError(f"Invalid PE header offset in {binary}")
        stream.seek(pe_offset)
        signature = stream.read(4)
        machine_bytes = stream.read(2)
        if signature != b"PE\0\0" or len(machine_bytes) != 2:
            raise PackageError(f"Missing or truncated PE header: {binary}")
        return struct.unpack("<H", machine_bytes)[0]


def _module_info(bundle: Path) -> tuple[dict, list[dict], Path]:
    info_path = bundle / "Contents" / "Resources" / "moduleinfo.json"
    if not info_path.is_file():
        raise PackageError(f"Missing VST3 moduleinfo.json: {info_path}")
    try:
        info = json.loads(info_path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise PackageError(f"Invalid VST3 moduleinfo.json: {exc}") from exc
    if not isinstance(info, dict):
        raise PackageError("VST3 moduleinfo.json must contain a JSON object")

    name = info.get("Name")
    version = info.get("Version")
    classes = info.get("Classes")
    if not isinstance(name, str) or not name.strip():
        raise PackageError("VST3 moduleinfo.json is missing a non-empty Name")
    if not isinstance(version, str) or not version.strip():
        raise PackageError("VST3 moduleinfo.json is missing a non-empty Version")
    if not isinstance(classes, list) or not classes:
        raise PackageError("VST3 moduleinfo.json must declare at least one class")

    seen: set[str] = set()
    normalized_classes: list[dict] = []
    for index, entry in enumerate(classes):
        if not isinstance(entry, dict):
            raise PackageError(f"VST3 class {index} is not an object")
        cid = entry.get("CID")
        if not isinstance(cid, str) or not re.fullmatch(r"[0-9a-fA-F]{32}", cid):
            raise PackageError(f"VST3 class {index} has an invalid 32-hex CID")
        normalized = cid.lower()
        if normalized in seen:
            raise PackageError(f"Duplicate VST3 class CID: {cid}")
        seen.add(normalized)
        normalized_classes.append({
            "cid": cid,
            "name": entry.get("Name"),
            "category": entry.get("Category"),
        })
    return info, normalized_classes, info_path


def _bundle_files(bundle: Path) -> list[Path]:
    for path in bundle.rglob("*"):
        if path.is_symlink():
            raise PackageError(f"VST3 bundle contains a symlink: {path}")
    files = sorted(p for p in bundle.rglob("*") if p.is_file())
    if not files:
        raise PackageError(f"VST3 bundle is empty: {bundle}")
    return files


def _validate_bundle(bundle: Path) -> tuple[dict, list[dict], Path, list[Path]]:
    if not bundle.is_dir() or bundle.suffix.lower() != ".vst3":
        raise PackageError(f"Not a VST3 bundle directory: {bundle}")
    info, classes, info_path = _module_info(bundle)
    binary_dir = bundle / "Contents" / "x86_64-win"
    binaries = sorted(p for p in binary_dir.glob("*.vst3") if p.is_file()) if binary_dir.is_dir() else []
    if len(binaries) != 1:
        raise PackageError(
            f"Expected exactly one x86_64 Windows VST3 module in {binary_dir}; found {len(binaries)}"
        )
    machine = _read_pe_machine(binaries[0])
    if machine != 0x8664:
        raise PackageError(
            f"Expected AMD64 PE machine 0x8664, found 0x{machine:04x}: {binaries[0]}"
        )
    files = _bundle_files(bundle)
    if info_path not in files or binaries[0] not in files:
        raise PackageError("VST3 metadata or module binary is not part of the bundle file set")
    return info, classes, binaries[0], files


def package_vst3(build_root: Path, output: Path) -> dict:
    build_root = build_root.resolve()
    output = output.resolve()
    if not build_root.is_dir():
        raise PackageError(f"Build root does not exist: {build_root}")
    if output.exists():
        raise PackageError(f"Refusing to overwrite an existing package: {output}")
    manifest_path = output.with_suffix(".manifest.json")
    checksum_path = output.with_suffix(".sha256")
    for sidecar in (manifest_path, checksum_path):
        if sidecar.exists():
            raise PackageError(f"Refusing to overwrite an existing sidecar: {sidecar}")

    bundles = sorted(p for p in build_root.rglob("*.vst3") if p.is_dir())
    if len(bundles) != 1:
        raise PackageError(
            f"Expected exactly one VST3 bundle below {build_root}; found {len(bundles)}"
        )
    bundle = bundles[0]
    info, classes, binary, files = _validate_bundle(bundle)

    output.parent.mkdir(parents=True, exist_ok=True)
    temp_output = output.with_name(output.name + ".tmp")
    if temp_output.exists():
        temp_output.unlink()

    file_records = []
    try:
        with zipfile.ZipFile(
            temp_output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9
        ) as archive:
            for path in files:
                relative = path.relative_to(bundle).as_posix()
                archive_name = f"{bundle.name}/{relative}"
                archive.write(path, archive_name)
                file_records.append({
                    "path": archive_name,
                    "size": path.stat().st_size,
                    "sha256": sha256_file(path),
                })

        with zipfile.ZipFile(temp_output, "r") as archive:
            if archive.testzip() is not None:
                raise PackageError("Packaged VST3 ZIP failed its CRC check")
            names = archive.namelist()
            if not names or any(name.startswith("/") or ".." in Path(name).parts for name in names):
                raise PackageError("Package contains an unsafe archive path")
            roots = {Path(name).parts[0] for name in names}
            if roots != {bundle.name}:
                raise PackageError(f"Package must contain only {bundle.name}; found {sorted(roots)}")
            if len(names) != len(file_records):
                raise PackageError("Package entry count differs from verified bundle file count")
            for record in file_records:
                with archive.open(record["path"], "r") as stream:
                    if sha256_bytes(stream.read()) != record["sha256"]:
                        raise PackageError(f"Archive content hash mismatch: {record['path']}")

        temp_output.replace(output)
    except Exception:
        if temp_output.exists():
            temp_output.unlink()
        raise

    archive_hash = sha256_file(output)
    binary_record = next(
        record for record in file_records
        if record["path"] == f"{bundle.name}/{binary.relative_to(bundle).as_posix()}"
    )
    factory = info.get("Factory Info", {})
    manifest = {
        "schema_version": 1,
        "product": info["Name"],
        "version": info["Version"],
        "format": "VST3",
        "platform": "windows-x64",
        "architecture": "AMD64",
        "bundle": bundle.name,
        "binary": binary_record["path"],
        "binary_sha256": binary_record["sha256"],
        "archive": output.name,
        "archive_size_bytes": output.stat().st_size,
        "archive_sha256": archive_hash,
        "bundle_file_count": len(file_records),
        "classes": classes,
        "vendor": factory.get("Vendor") if isinstance(factory, dict) else None,
        "source_commit": os.environ.get("GITHUB_SHA"),
        "workflow_run_id": os.environ.get("GITHUB_RUN_ID"),
        "workflow_attempt": os.environ.get("GITHUB_RUN_ATTEMPT"),
        "files": file_records,
    }
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    checksum_path.write_text(f"{archive_hash}  {output.name}\n", encoding="ascii")
    return manifest


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        manifest = package_vst3(args.build_root, args.output)
    except PackageError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    print(
        "PASS: packaged one Windows x64 VST3 bundle "
        f"{manifest['bundle']} v{manifest['version']} "
        f"({manifest['bundle_file_count']} files, SHA256 {manifest['archive_sha256']})"
    )
    print(f"Manifest: {args.output.with_suffix('.manifest.json')}")
    print(f"Checksum: {args.output.with_suffix('.sha256')}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
