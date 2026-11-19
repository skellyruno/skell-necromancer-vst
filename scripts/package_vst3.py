#!/usr/bin/env python3
"""Normalize and verify the Windows VST3 bundle before creating a preview ZIP.

JUCE's moduleinfo.json may contain JSON5-style trailing commas. The package
shipped to users is rewritten as canonical strict JSON, then checked against
this product's two-class VST3 contract and AMD64 PE32+ binary requirement.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import zipfile
from pathlib import Path
from typing import Any


class PackageError(RuntimeError):
    """Raised when a build is not a valid distributable VST3 package."""


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def strip_json5_trailing_commas(source: str) -> str:
    """Remove commas immediately before } or ] without changing string data."""
    output: list[str] = []
    in_string = False
    escaped = False
    index = 0

    while index < len(source):
        char = source[index]
        if in_string:
            output.append(char)
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            index += 1
            continue

        if char == '"':
            in_string = True
            output.append(char)
            index += 1
            continue

        if char == ",":
            lookahead = index + 1
            while lookahead < len(source) and source[lookahead].isspace():
                lookahead += 1
            if lookahead < len(source) and source[lookahead] in "]}":
                index += 1
                continue

        output.append(char)
        index += 1

    if in_string:
        raise PackageError("moduleinfo.json contains an unterminated string")
    return "".join(output)


def load_and_normalize_moduleinfo(path: Path) -> dict[str, Any]:
    """Parse JUCE metadata and atomically rewrite it as strict UTF-8 JSON."""
    try:
        source = path.read_text(encoding="utf-8-sig")
    except OSError as error:
        raise PackageError(f"cannot read {path}: {error}") from error

    normalized = strip_json5_trailing_commas(source)
    try:
        data = json.loads(normalized)
    except json.JSONDecodeError as error:
        raise PackageError(
            f"moduleinfo.json is not valid JSON after trailing-comma normalization: {error}"
        ) from error

    if not isinstance(data, dict):
        raise PackageError("moduleinfo.json root must be an object")

    canonical = json.dumps(data, ensure_ascii=False, indent=2) + "\n"
    temporary = path.with_name(path.name + ".tmp")
    try:
        with temporary.open("w", encoding="utf-8", newline="\n") as stream:
            stream.write(canonical)
        temporary.replace(path)
    except OSError as error:
        temporary.unlink(missing_ok=True)
        raise PackageError(f"cannot write normalized moduleinfo.json: {error}") from error
    return data


def validate_metadata(
    metadata: dict[str, Any], *, expected_name: str, expected_version: str, expected_vendor: str
) -> None:
    if metadata.get("Name") != expected_name:
        raise PackageError(f"unexpected VST3 product name: {metadata.get('Name')!r}")
    if metadata.get("Version") != expected_version:
        raise PackageError(f"VST3 version must be {expected_version!r}")

    factory = metadata.get("Factory Info")
    if not isinstance(factory, dict) or factory.get("Vendor") != expected_vendor:
        raise PackageError(f"Factory Info vendor must be {expected_vendor!r}")
    for field in ("URL", "E-Mail"):
        if not isinstance(factory.get(field), str):
            raise PackageError(f"Factory Info {field} must be a string (empty is allowed for preview)")

    classes = metadata.get("Classes")
    if not isinstance(classes, list) or len(classes) != 2:
        raise PackageError("VST3 metadata must contain exactly two class records")

    expected_categories = {"Audio Module Class", "Component Controller Class"}
    categories: set[str] = set()
    class_ids: set[str] = set()
    for record in classes:
        if not isinstance(record, dict):
            raise PackageError("each VST3 class record must be an object")
        category = record.get("Category")
        if not isinstance(category, str):
            raise PackageError("VST3 class Category must be a string")
        categories.add(category)

        cid = record.get("CID")
        if not isinstance(cid, str) or re.fullmatch(r"[0-9A-Fa-f]{32}", cid) is None:
            raise PackageError(f"invalid 32-hex VST3 CID: {cid!r}")
        if cid.lower() in class_ids:
            raise PackageError("VST3 class CIDs must be unique")
        class_ids.add(cid.lower())

        if record.get("Name") != expected_name:
            raise PackageError("VST3 class name does not match the product")
        if record.get("Vendor") != expected_vendor:
            raise PackageError("VST3 class vendor does not match Factory Info")
        if record.get("Version") != expected_version:
            raise PackageError("VST3 class version does not match the product version")

    if categories != expected_categories:
        raise PackageError(
            "VST3 classes must be exactly one Audio Module Class and one Component Controller Class"
        )


def verify_amd64_pe32_plus(binary: Path) -> None:
    """Require a Windows AMD64 PE32+ VST3 module, not merely an MZ stub."""
    try:
        with binary.open("rb") as stream:
            dos = stream.read(64)
            if len(dos) < 64 or dos[:2] != b"MZ":
                raise PackageError("VST3 module is not a valid DOS/PE executable")
            pe_offset = struct.unpack_from("<I", dos, 0x3C)[0]
            if pe_offset < 64 or pe_offset > 1024 * 1024:
                raise PackageError("VST3 module has an invalid PE header offset")
            stream.seek(pe_offset)
            header = stream.read(26)
    except OSError as error:
        raise PackageError(f"cannot read VST3 module {binary}: {error}") from error

    if len(header) != 26 or header[:4] != b"PE\0\0":
        raise PackageError("VST3 module is missing its PE signature")
    machine = struct.unpack_from("<H", header, 4)[0]
    optional_magic = struct.unpack_from("<H", header, 24)[0]
    if machine != 0x8664 or optional_magic != 0x20B:
        raise PackageError(
            f"VST3 module must be AMD64 PE32+ (machine=0x8664, optional=0x20b); "
            f"found machine=0x{machine:04x}, optional=0x{optional_magic:04x}"
        )


def package_vst3(
    *,
    build_dir: Path,
    expected_bundle: Path,
    output_zip: Path,
    manifest_path: Path,
    expected_name: str,
    expected_version: str,
    expected_vendor: str,
) -> dict[str, Any]:
    build_dir = build_dir.resolve()
    expected_bundle = expected_bundle.resolve()
    if not build_dir.is_dir():
        raise PackageError(f"build directory does not exist: {build_dir}")
    if not expected_bundle.is_dir() or expected_bundle.suffix.lower() != ".vst3":
        raise PackageError(f"expected VST3 bundle does not exist: {expected_bundle}")

    bundles = sorted(path.resolve() for path in build_dir.rglob("*.vst3") if path.is_dir())
    if bundles != [expected_bundle]:
        found = ", ".join(str(path) for path in bundles) or "none"
        raise PackageError(f"expected exactly one VST3 bundle at {expected_bundle}; found: {found}")

    moduleinfo = expected_bundle / "Contents" / "Resources" / "moduleinfo.json"
    binary = expected_bundle / "Contents" / "x86_64-win" / f"{expected_name}.vst3"
    if not moduleinfo.is_file():
        raise PackageError(f"missing VST3 module metadata: {moduleinfo}")
    if not binary.is_file():
        raise PackageError(f"missing Windows x86_64 VST3 module: {binary}")

    modules = [path for path in expected_bundle.rglob("*.vst3") if path.is_file()]
    if modules != [binary]:
        raise PackageError("bundle must contain exactly one VST3 module binary")
    metadata_files = list(expected_bundle.rglob("moduleinfo.json"))
    if metadata_files != [moduleinfo]:
        raise PackageError("bundle must contain exactly one moduleinfo.json at the VST3 Resources path")

    metadata = load_and_normalize_moduleinfo(moduleinfo)
    validate_metadata(
        metadata,
        expected_name=expected_name,
        expected_version=expected_version,
        expected_vendor=expected_vendor,
    )
    verify_amd64_pe32_plus(binary)

    output_zip = output_zip.resolve()
    manifest_path = manifest_path.resolve()
    output_zip.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output_zip, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in sorted(p for p in expected_bundle.rglob("*") if p.is_file()):
            archive.write(path, path.relative_to(expected_bundle.parent).as_posix())

    with zipfile.ZipFile(output_zip, "r") as archive:
        bad_entry = archive.testzip()
        if bad_entry is not None:
            raise PackageError(f"packaged ZIP CRC failed at {bad_entry}")
        names = archive.namelist()
        prefix = expected_bundle.name + "/"
        if not names or any(not name.startswith(prefix) for name in names):
            raise PackageError("package ZIP must contain only the single expected VST3 bundle")
        archived_info = json.loads(archive.read(f"{prefix}Contents/Resources/moduleinfo.json"))
        validate_metadata(
            archived_info,
            expected_name=expected_name,
            expected_version=expected_version,
            expected_vendor=expected_vendor,
        )

    file_records = []
    for path in sorted(p for p in expected_bundle.rglob("*") if p.is_file()):
        file_records.append({
            "path": path.relative_to(expected_bundle).as_posix(),
            "size_bytes": path.stat().st_size,
            "sha256": sha256_file(path),
        })

    factory = metadata["Factory Info"]
    manifest = {
        "format": "VST3",
        "product": expected_name,
        "version": expected_version,
        "vendor": expected_vendor,
        "architecture": "x86_64-win",
        "bundle": expected_bundle.name,
        "preview_only": not bool(factory.get("URL") and factory.get("E-Mail")),
        "publisher_contact_configured": bool(factory.get("URL") and factory.get("E-Mail")),
        "moduleinfo_sha256": sha256_file(moduleinfo),
        "module_sha256": sha256_file(binary),
        "archive_sha256": sha256_file(output_zip),
        "files": file_records,
    }
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--expected-bundle", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--name", default="Skell-Necromancer")
    parser.add_argument("--version", default="0.1.0")
    parser.add_argument("--vendor", default="Skell Audio")
    args = parser.parse_args()

    try:
        manifest = package_vst3(
            build_dir=args.build_dir,
            expected_bundle=args.expected_bundle,
            output_zip=args.output,
            manifest_path=args.manifest,
            expected_name=args.name,
            expected_version=args.version,
            expected_vendor=args.vendor,
        )
    except (PackageError, OSError, zipfile.BadZipFile) as error:
        parser.error(str(error))

    print(
        "VST3 package verified: "
        f"{manifest['product']} {manifest['version']} "
        f"{manifest['architecture']} · SHA-256 {manifest['archive_sha256']}"
    )
    print(f"Preview only: {manifest['preview_only']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
