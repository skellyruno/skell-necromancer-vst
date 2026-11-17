from __future__ import annotations

import json
import struct
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools.package_vst3 import PackageError, package_vst3


def write_fake_pe64(path: Path) -> None:
    data = bytearray(512)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<H", data, 0x84, 0x8664)
    path.write_bytes(data)


def make_bundle(
    root: Path,
    name: str = "Skell-Necromancer.vst3",
    cid: str = "0123456789ABCDEF0123456789ABCDEF",
) -> Path:
    bundle = root / name
    info = bundle / "Contents" / "Resources" / "moduleinfo.json"
    binary = bundle / "Contents" / "x86_64-win" / "Skell-Necromancer.vst3"
    info.parent.mkdir(parents=True)
    binary.parent.mkdir(parents=True)
    info.write_text(json.dumps({
        "Name": "Skell-Necromancer",
        "Version": "0.1.0",
        "Factory Info": {"Vendor": "Skell Audio"},
        "Classes": [{"CID": cid, "Name": "Skell-Necromancer", "Category": "Audio Module Class"}],
    }), encoding="utf-8")
    write_fake_pe64(binary)
    (bundle / "Contents" / "Resources" / "readme.txt").write_text("bundle resource", encoding="utf-8")
    return bundle


class Vst3PackagerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.build = self.root / "build"
        self.build.mkdir()
        self.output = self.root / "Skell-Necromancer-Windows-VST3.zip"

    def tearDown(self) -> None:
        self.temp.cleanup()

    def test_packages_one_valid_x64_bundle_with_manifest_and_checksum(self) -> None:
        bundle = make_bundle(self.build)
        manifest = package_vst3(self.build, self.output)
        self.assertTrue(self.output.is_file())
        self.assertEqual(manifest["format"], "VST3")
        self.assertEqual(manifest["platform"], "windows-x64")
        self.assertEqual(manifest["architecture"], "AMD64")
        self.assertEqual(manifest["version"], "0.1.0")
        self.assertEqual(manifest["bundle_file_count"], 3)
        self.assertEqual(len(manifest["classes"]), 1)
        self.assertEqual(
            self.output.with_suffix(".sha256").read_text(encoding="ascii").split()[0],
            manifest["archive_sha256"],
        )
        with zipfile.ZipFile(self.output) as archive:
            self.assertIsNone(archive.testzip())
            names = archive.namelist()
            self.assertTrue(all(name.startswith(bundle.name + "/") for name in names))
            self.assertIn(bundle.name + "/Contents/Resources/moduleinfo.json", names)
            self.assertEqual(len(names), 3)

    def test_refuses_multiple_vst3_bundles_in_build_tree(self) -> None:
        make_bundle(self.build, "Skell-Necromancer.vst3")
        make_bundle(self.build, "Unexpected.vst3", "FEDCBA9876543210FEDCBA9876543210")
        with self.assertRaisesRegex(PackageError, "exactly one VST3 bundle"):
            package_vst3(self.build, self.output)

    def test_refuses_duplicate_class_ids(self) -> None:
        bundle = make_bundle(self.build)
        info = bundle / "Contents" / "Resources" / "moduleinfo.json"
        payload = json.loads(info.read_text(encoding="utf-8"))
        payload["Classes"].append(dict(payload["Classes"][0]))
        info.write_text(json.dumps(payload), encoding="utf-8")
        with self.assertRaisesRegex(PackageError, "Duplicate VST3 class CID"):
            package_vst3(self.build, self.output)

    def test_refuses_non_amd64_binary(self) -> None:
        bundle = make_bundle(self.build)
        binary = bundle / "Contents" / "x86_64-win" / "Skell-Necromancer.vst3"
        data = bytearray(binary.read_bytes())
        struct.pack_into("<H", data, 0x84, 0x14C)
        binary.write_bytes(data)
        with self.assertRaisesRegex(PackageError, "Expected AMD64 PE machine"):
            package_vst3(self.build, self.output)

    def test_refuses_missing_moduleinfo(self) -> None:
        bundle = make_bundle(self.build)
        (bundle / "Contents" / "Resources" / "moduleinfo.json").unlink()
        with self.assertRaisesRegex(PackageError, "Missing VST3 moduleinfo"):
            package_vst3(self.build, self.output)

    def test_refuses_missing_binary(self) -> None:
        bundle = make_bundle(self.build)
        (bundle / "Contents" / "x86_64-win" / "Skell-Necromancer.vst3").unlink()
        with self.assertRaisesRegex(PackageError, "exactly one x86_64 Windows VST3 module"):
            package_vst3(self.build, self.output)

    def test_refuses_invalid_moduleinfo_json(self) -> None:
        bundle = make_bundle(self.build)
        (bundle / "Contents" / "Resources" / "moduleinfo.json").write_text("{", encoding="utf-8")
        with self.assertRaisesRegex(PackageError, "Invalid VST3 moduleinfo"):
            package_vst3(self.build, self.output)

    def test_refuses_existing_sidecar_before_creating_package(self) -> None:
        make_bundle(self.build)
        self.output.with_suffix(".manifest.json").write_text("keep", encoding="utf-8")
        with self.assertRaisesRegex(PackageError, "Refusing to overwrite an existing sidecar"):
            package_vst3(self.build, self.output)
        self.assertFalse(self.output.exists())

    def test_refuses_to_overwrite_existing_package(self) -> None:
        make_bundle(self.build)
        self.output.write_bytes(b"known-good package")
        with self.assertRaisesRegex(PackageError, "Refusing to overwrite"):
            package_vst3(self.build, self.output)
        self.assertEqual(self.output.read_bytes(), b"known-good package")


if __name__ == "__main__":
    unittest.main(verbosity=2)
