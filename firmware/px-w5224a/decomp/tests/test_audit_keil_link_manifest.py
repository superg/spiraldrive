import tempfile
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))

from audit_keil_link_manifest import audit


class KeilLinkManifestAuditTests(unittest.TestCase):
    def test_manifest_and_unresolved_symbol_classification(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            objects = root / "firmware"
            sources = root / "src"
            objects.mkdir()
            sources.mkdir()
            manifest = objects / "link_manifest.txt"
            manifest.write_text("one.obj\ntwo.obj\n")
            for name in ("one.obj", "two.obj"):
                (objects / name).write_bytes(b"object")
            (objects / "one.lst").write_text(
                "; FUNCTION NearDefinition (BEGIN)\n")
            (sources / "source_only.c").write_text(
                "void SourceOnlyDefinition(void) {}\n")
            link_log = objects / "link.log"
            link_log.write_text(
                "one.obj,two.obj TO main.abs\n"
                "*** ERROR L127: UNRESOLVED EXTERNAL SYMBOL\n"
                "    SYMBOL:  ?C?FPADD?\n"
                "*** ERROR L127: UNRESOLVED EXTERNAL SYMBOL\n"
                "    SYMBOL:  NearDefinition?\n"
                "*** ERROR L127: UNRESOLVED EXTERNAL SYMBOL\n"
                "    SYMBOL:  SourceOnlyDefinition?\n")

            report = audit(manifest, objects, link_log, sources)

            self.assertEqual(report.manifest_entries, 2)
            self.assertEqual(report.link_entries, 2)
            self.assertEqual(report.missing_objects, [])
            self.assertEqual(report.unresolved_compiler_runtime,
                             ["?C?FPADD?"])
            self.assertEqual(
                report.unresolved_decoration_mismatches,
                {"NearDefinition?": ["NearDefinition"]})
            self.assertEqual(
                report.unresolved_source_without_compiled_definition,
                ["SourceOnlyDefinition?"])

    def test_duplicate_and_missing_manifest_entries_are_reported(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            manifest = root / "link_manifest.txt"
            manifest.write_text("one.obj\none.obj\nmissing.obj\n")
            (root / "one.obj").write_bytes(b"object")

            report = audit(manifest, root, None, None)

            self.assertEqual(report.manifest_duplicates, {"one.obj": 2})
            self.assertEqual(report.missing_objects, ["missing.obj"])


if __name__ == "__main__":
    unittest.main()
