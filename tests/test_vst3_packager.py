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
        self.source = self.root / "source"
        font_dir = self.source / "Assets" / "fonts"
        font_dir.mkdir(parents=True)
        (font_dir / "Montserrat-SemiBold.ttf").write_bytes(b"embedded-font-fixture")
        (font_dir / "OFL-Montserrat.txt").write_text(
            "SIL OPEN FONT LICENSE Version 1.1\\nfixture license text\\n",
            encoding="utf-8",
        )

    def package(self) -> dict:
        return package_vst3(self.build, self.output, self.source)

    def tearDown(self) -> None:
        self.temp.cleanup()

    def test_packages_one_valid_x64_bundle_with_manifest_and_checksum(self) -> None:
        bundle = make_bundle(self.build)
        manifest = self.package()
        self.assertTrue(self.output.is_file())
        self.assertEqual(manifest["format"], "VST3")
        self.assertEqual(manifest["platform"], "windows-x64")
        self.assertEqual(manifest["architecture"], "AMD64")
        self.assertEqual(manifest["version"], "0.1.0")
        self.assertEqual(manifest["bundle_file_count"], 3)
        self.assertEqual(manifest["package_file_count"], 4)
        self.assertEqual(
            manifest["third_party_notices"],
            ["THIRD_PARTY_NOTICES/OFL-Montserrat.txt"],
        )
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


    def test_normalizes_juce_trailing_commas_in_package_only(self) -> None:
        bundle = make_bundle(self.build)
        info_path = bundle / "Contents" / "Resources" / "moduleinfo.json"
        original = info_path.read_text(encoding="utf-8")
        payload = json.loads(original)
        raw = json.dumps(payload, indent=2).replace(
            '"Category": "Audio Module Class"\n    }',
            '"Category": "Audio Module Class",\n    }',
        )
        info_path.write_text(raw, encoding="utf-8")
        original_hash = __import__("hashlib").sha256(raw.encode("utf-8")).hexdigest()

        manifest = self.package()

        self.assertTrue(manifest["moduleinfo_normalized"])
        self.assertEqual(manifest["moduleinfo_original_sha256"], original_hash)
        self.assertNotEqual(manifest["moduleinfo_sha256"], original_hash)
        self.assertEqual(info_path.read_text(encoding="utf-8"), raw)
        with zipfile.ZipFile(self.output) as archive:
            packaged_text = archive.read(
                bundle.name + "/Contents/Resources/moduleinfo.json"
            ).decode("utf-8")
            parsed = json.loads(packaged_text)
            self.assertEqual(parsed["Name"], "Skell-Necromancer")
            self.assertNotRegex(packaged_text, r",\s*[}\]]")

    def test_refuses_multiple_vst3_bundles_in_build_tree(self) -> None:
        make_bundle(self.build, "Skell-Necromancer.vst3")
        make_bundle(self.build, "Unexpected.vst3", "FEDCBA9876543210FEDCBA9876543210")
        with self.assertRaisesRegex(PackageError, "exactly one VST3 bundle"):
            self.package()

    def test_refuses_duplicate_class_ids(self) -> None:
        bundle = make_bundle(self.build)
        info = bundle / "Contents" / "Resources" / "moduleinfo.json"
        payload = json.loads(info.read_text(encoding="utf-8"))
        payload["Classes"].append(dict(payload["Classes"][0]))
        info.write_text(json.dumps(payload), encoding="utf-8")
        with self.assertRaisesRegex(PackageError, "Duplicate VST3 class CID"):
            self.package()

    def test_refuses_non_amd64_binary(self) -> None:
        bundle = make_bundle(self.build)
        binary = bundle / "Contents" / "x86_64-win" / "Skell-Necromancer.vst3"
        data = bytearray(binary.read_bytes())
        struct.pack_into("<H", data, 0x84, 0x14C)
        binary.write_bytes(data)
        with self.assertRaisesRegex(PackageError, "Expected AMD64 PE machine"):
            self.package()

    def test_refuses_missing_moduleinfo(self) -> None:
        bundle = make_bundle(self.build)
        (bundle / "Contents" / "Resources" / "moduleinfo.json").unlink()
        with self.assertRaisesRegex(PackageError, "Missing VST3 moduleinfo"):
            self.package()

    def test_refuses_missing_binary(self) -> None:
        bundle = make_bundle(self.build)
        (bundle / "Contents" / "x86_64-win" / "Skell-Necromancer.vst3").unlink()
        with self.assertRaisesRegex(PackageError, "exactly one x86_64 Windows VST3 module"):
            self.package()

    def test_refuses_invalid_moduleinfo_json(self) -> None:
        bundle = make_bundle(self.build)
        (bundle / "Contents" / "Resources" / "moduleinfo.json").write_text("{", encoding="utf-8")
        with self.assertRaisesRegex(PackageError, "Invalid VST3 moduleinfo"):
            self.package()

    def test_refuses_existing_sidecar_before_creating_package(self) -> None:
        make_bundle(self.build)
        self.output.with_suffix(".manifest.json").write_text("keep", encoding="utf-8")
        with self.assertRaisesRegex(PackageError, "Refusing to overwrite an existing sidecar"):
            self.package()
        self.assertFalse(self.output.exists())

    def test_requires_font_license_when_font_is_embedded(self) -> None:
        make_bundle(self.build)
        (self.source / "Assets" / "fonts" / "OFL-Montserrat.txt").unlink()
        with self.assertRaisesRegex(PackageError, "Montserrat is bundled"):
            self.package()
        self.assertFalse(self.output.exists())

    def test_refuses_to_overwrite_existing_package(self) -> None:
        make_bundle(self.build)
        self.output.write_bytes(b"known-good package")
        with self.assertRaisesRegex(PackageError, "Refusing to overwrite"):
            self.package()
        self.assertEqual(self.output.read_bytes(), b"known-good package")


if __name__ == "__main__":
    unittest.main(verbosity=2)
