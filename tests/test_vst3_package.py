from __future__ import annotations

import json
import struct
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from package_vst3 import (  # noqa: E402
    PackageError,
    load_and_normalize_moduleinfo,
    package_vst3,
    strip_json5_trailing_commas,
)


PRODUCT = "Skell-Necromancer"
VERSION = "0.1.0"
VENDOR = "Skell Audio"


def metadata_text(*, duplicate_cid: bool = False) -> str:
    second_cid = "ABCDEF019182FAEB536B4175536E4372" if duplicate_cid else "ABCDEF011234ABCD536B4175536E4372"
    return f'''{{
  "Name": "{PRODUCT}",
  "Version": "{VERSION}",
  "Factory Info": {{
    "Vendor": "{VENDOR}",
    "URL": "",
    "E-Mail": "",
    "Flags": {{ "Unicode": true, }},
  }},
  "Classes": [
    {{ "CID": "ABCDEF019182FAEB536B4175536E4372", "Category": "Audio Module Class", "Name": "{PRODUCT}", "Vendor": "{VENDOR}", "Version": "{VERSION}", }},
    {{ "CID": "{second_cid}", "Category": "Component Controller Class", "Name": "{PRODUCT}", "Vendor": "{VENDOR}", "Version": "{VERSION}", }},
  ],
}}
'''


def write_fake_amd64_pe(path: Path, machine: int = 0x8664, optional_magic: int = 0x20B) -> None:
    data = bytearray(512)
    data[:2] = b"MZ"
    pe_offset = 0x80
    struct.pack_into("<I", data, 0x3C, pe_offset)
    data[pe_offset:pe_offset + 4] = b"PE\0\0"
    struct.pack_into("<H", data, pe_offset + 4, machine)
    struct.pack_into("<H", data, pe_offset + 24, optional_magic)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def make_bundle(root: Path, *, duplicate_cid: bool = False, machine: int = 0x8664) -> tuple[Path, Path]:
    build = root / "build"
    bundle = build / "SkellNecromancer_artefacts" / "Release" / "VST3" / f"{PRODUCT}.vst3"
    info = bundle / "Contents" / "Resources" / "moduleinfo.json"
    binary = bundle / "Contents" / "x86_64-win" / f"{PRODUCT}.vst3"
    info.parent.mkdir(parents=True, exist_ok=True)
    info.write_text(metadata_text(duplicate_cid=duplicate_cid), encoding="utf-8")
    write_fake_amd64_pe(binary, machine=machine)
    return build, bundle


class Vst3PackagingTests(unittest.TestCase):
    def test_trailing_commas_are_removed_outside_strings_only(self) -> None:
        source = r'{"text":"literal ,} plus \"quoted\" and C:\\temp", "nested":[1,2,],}'
        normalized = strip_json5_trailing_commas(source)
        value = json.loads(normalized)
        self.assertEqual(value["text"], 'literal ,} plus "quoted" and C:\\temp')
        self.assertEqual(value["nested"], [1, 2])

    def test_normalizer_writes_strict_canonical_json(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "moduleinfo.json"
            path.write_text(metadata_text(), encoding="utf-8")
            data = load_and_normalize_moduleinfo(path)
            reread = json.loads(path.read_text(encoding="utf-8"))
            self.assertEqual(data, reread)
            self.assertNotIn(",\n  }", path.read_text(encoding="utf-8"))
            self.assertNotIn(",\n  ]", path.read_text(encoding="utf-8"))

    def test_malformed_json_is_rejected_without_rewriting(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "moduleinfo.json"
            original = '{"Name": "broken",,}'
            path.write_text(original, encoding="utf-8")
            with self.assertRaises(PackageError):
                load_and_normalize_moduleinfo(path)
            self.assertEqual(path.read_text(encoding="utf-8"), original)

    def test_package_normalizes_metadata_and_emits_hash_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            build, bundle = make_bundle(root)
            output = root / "Skell-Necromancer-Windows-VST3.zip"
            manifest_path = root / "Skell-Necromancer-Windows-VST3.manifest.json"
            manifest = package_vst3(
                build_dir=build,
                expected_bundle=bundle,
                output_zip=output,
                manifest_path=manifest_path,
                expected_name=PRODUCT,
                expected_version=VERSION,
                expected_vendor=VENDOR,
            )
            self.assertTrue(manifest["preview_only"])
            self.assertEqual(manifest["architecture"], "x86_64-win")
            self.assertEqual(manifest["archive_sha256"], json.loads(manifest_path.read_text())["archive_sha256"])
            with zipfile.ZipFile(output) as archive:
                self.assertIsNone(archive.testzip())
                names = archive.namelist()
                self.assertTrue(names)
                self.assertTrue(all(name.startswith(f"{PRODUCT}.vst3/") for name in names))
                json.loads(archive.read(f"{PRODUCT}.vst3/Contents/Resources/moduleinfo.json"))

    def test_duplicate_class_ids_are_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            build, bundle = make_bundle(Path(folder), duplicate_cid=True)
            with self.assertRaisesRegex(PackageError, "CIDs must be unique"):
                package_vst3(
                    build_dir=build,
                    expected_bundle=bundle,
                    output_zip=Path(folder) / "out.zip",
                    manifest_path=Path(folder) / "manifest.json",
                    expected_name=PRODUCT,
                    expected_version=VERSION,
                    expected_vendor=VENDOR,
                )

    def test_non_amd64_module_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            build, bundle = make_bundle(Path(folder), machine=0x14C)
            with self.assertRaisesRegex(PackageError, r"AMD64 PE32\+"):
                package_vst3(
                    build_dir=build,
                    expected_bundle=bundle,
                    output_zip=Path(folder) / "out.zip",
                    manifest_path=Path(folder) / "manifest.json",
                    expected_name=PRODUCT,
                    expected_version=VERSION,
                    expected_vendor=VENDOR,
                )

    def test_duplicate_vst3_bundles_are_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            build, bundle = make_bundle(Path(folder))
            duplicate = build / "unexpected" / "Duplicate.vst3"
            duplicate.mkdir(parents=True)
            with self.assertRaisesRegex(PackageError, "exactly one VST3 bundle"):
                package_vst3(
                    build_dir=build,
                    expected_bundle=bundle,
                    output_zip=Path(folder) / "out.zip",
                    manifest_path=Path(folder) / "manifest.json",
                    expected_name=PRODUCT,
                    expected_version=VERSION,
                    expected_vendor=VENDOR,
                )


if __name__ == "__main__":
    unittest.main()
