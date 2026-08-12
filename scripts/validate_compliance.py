#!/usr/bin/env python3
"""Validate a completed Stack Junkie visual compliance report.

This checker is intentionally strict about completeness and evidence linkage.
It cannot decide whether a screenshot actually looks correct. In independent
mode, that judgment belongs to the visual verifier described by the skill. In
self mode, the output is explicitly limited to a self-audit claim.
"""

from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path


CHECK_RE = re.compile(
    r"^- \[(?P<checked>[ xX])\] "
    r"(?P<id>SJ-[A-Z0-9]+-\d{2}) "
    r"(?P<label>.*?) \| Result: (?P<result>PASS|FAIL|N/A|TODO) "
    r"\| Evidence: (?P<evidence>.*?) \| Notes: (?P<notes>.*)$"
)
ID_RE = re.compile(r"^- \[[ xX]\] (SJ-[A-Z0-9]+-\d{2}) ", re.MULTILINE)
EVIDENCE_ID_RE = re.compile(r"\bE\d{2,}\b")
IMAGE_SUFFIXES = {".png", ".jpg", ".jpeg", ".webp"}


@dataclass(frozen=True)
class Evidence:
    evidence_id: str
    file: str
    phase: str
    theme: str
    surface: str
    state: str
    size: str
    viewport: str
    purpose: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Validate a completed Stack Junkie compliance report."
    )
    parser.add_argument("report", type=Path, help="Path to the completed Markdown report")
    parser.add_argument(
        "--audit-level",
        choices=("independent", "self"),
        default="independent",
        help="Claim level to validate. Default: independent.",
    )
    return parser.parse_args()


def canonical_template() -> Path:
    return Path(__file__).resolve().parents[1] / "assets" / "templates" / "compliance-report.md"


def expected_ids(template_text: str) -> list[str]:
    return ID_RE.findall(template_text)


def parse_manifest(text: str) -> tuple[dict[str, Evidence], list[str]]:
    evidence: dict[str, Evidence] = {}
    errors: list[str] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        if not re.match(r"^\| E\d{2,} \|", line):
            continue
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) != 9:
            errors.append(
                f"line {line_number}: evidence row must contain 9 columns, found {len(cells)}"
            )
            continue
        item = Evidence(*cells)
        if item.evidence_id in evidence:
            errors.append(f"line {line_number}: duplicate evidence ID {item.evidence_id}")
            continue
        evidence[item.evidence_id] = item
    return evidence, errors


def parse_checks(text: str) -> tuple[dict[str, re.Match[str]], list[str]]:
    matches: dict[str, re.Match[str]] = {}
    errors: list[str] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        if not line.startswith("- [") or "SJ-" not in line:
            continue
        match = CHECK_RE.match(line)
        if not match:
            errors.append(f"line {line_number}: malformed checklist row")
            continue
        check_id = match.group("id")
        if check_id in matches:
            errors.append(f"line {line_number}: duplicate checklist ID {check_id}")
            continue
        matches[check_id] = match
    return matches, errors


def metadata_value(text: str, label: str) -> str | None:
    match = re.search(rf"^{re.escape(label)}:\s*(.+?)\s*$", text, re.MULTILINE)
    return match.group(1).strip() if match else None


def resolve_evidence_path(report: Path, raw_file: str) -> Path:
    value = raw_file.strip()
    markdown_link = re.fullmatch(r"\[[^\]]+\]\((.+)\)", value)
    if markdown_link:
        value = markdown_link.group(1).strip().strip("<>")
    path = Path(value)
    if not path.is_absolute():
        path = report.parent / path
    return path.resolve()


def main() -> int:
    args = parse_args()
    report = args.report.resolve()
    template = canonical_template()
    errors: list[str] = []

    if not report.is_file():
        print(f"ERROR: report not found: {report}", file=sys.stderr)
        return 2
    if not template.is_file():
        print(f"ERROR: canonical template not found: {template}", file=sys.stderr)
        return 2

    text = report.read_text(encoding="utf-8")
    template_text = template.read_text(encoding="utf-8")
    expected = expected_ids(template_text)

    if not expected:
        errors.append("canonical template contains no checklist IDs")
    expected_counts = Counter(expected)
    duplicates_in_template = sorted(key for key, count in expected_counts.items() if count > 1)
    if duplicates_in_template:
        errors.append("canonical template has duplicate IDs: " + ", ".join(duplicates_in_template))

    checks, check_errors = parse_checks(text)
    evidence, manifest_errors = parse_manifest(text)
    errors.extend(check_errors)
    errors.extend(manifest_errors)

    expected_set = set(expected)
    actual_set = set(checks)
    missing = sorted(expected_set - actual_set)
    unexpected = sorted(actual_set - expected_set)
    if missing:
        errors.append("missing checklist IDs: " + ", ".join(missing))
    if unexpected:
        errors.append("unexpected checklist IDs: " + ", ".join(unexpected))

    required_metadata = [
        "Project",
        "Scope",
        "Date",
        "Implementer",
        "Audit level",
        "Independent verifier",
        "Independent verification",
        "Theme source",
        "Evidence directory",
    ]
    metadata: dict[str, str] = {}
    for label in required_metadata:
        value = metadata_value(text, label)
        if not value or value.upper() in {"TODO", "N/A"}:
            errors.append(f"metadata field '{label}' is missing or incomplete")
        else:
            metadata[label] = value

    report_audit_level = metadata.get("Audit level", "").casefold()
    if report_audit_level and report_audit_level != args.audit_level:
        errors.append(
            f"report Audit level is '{report_audit_level}', but --audit-level is "
            f"'{args.audit_level}'"
        )

    independent_status = metadata.get("Independent verification", "").upper()
    verifier_label = metadata.get("Independent verifier", "").casefold()
    if args.audit_level == "independent":
        if independent_status != "PASS":
            errors.append("Independent verification must be PASS for an independent report")
        if verifier_label in {"unavailable", "not available", "not run"}:
            errors.append("Independent verifier must name the verifier for an independent report")
    else:
        if independent_status != "NOT RUN":
            errors.append("Independent verification must be NOT RUN for a self-audit report")
        if verifier_label not in {"unavailable", "not available", "not run"}:
            errors.append(
                "Independent verifier must be Unavailable for a self-audit report"
            )
    implementer = metadata.get("Implementer", "").casefold()
    verifier = metadata.get("Independent verifier", "").casefold()
    if implementer and verifier and implementer == verifier:
        errors.append("Implementer and Independent verifier must be different")

    if "TODO" in text:
        errors.append("report still contains TODO placeholders")

    for check_id in sorted(expected_set & actual_set):
        match = checks[check_id]
        checked = match.group("checked").lower() == "x"
        result = match.group("result")
        evidence_text = match.group("evidence").strip()
        notes = match.group("notes").strip()

        if not checked:
            errors.append(f"{check_id}: item was not checked")
        if result in {"TODO", "FAIL"}:
            errors.append(f"{check_id}: result is {result}")
            continue
        if result == "PASS":
            cited = EVIDENCE_ID_RE.findall(evidence_text)
            if not cited:
                errors.append(f"{check_id}: PASS requires at least one evidence ID")
            for evidence_id in cited:
                if evidence_id not in evidence:
                    errors.append(f"{check_id}: cites missing evidence {evidence_id}")
        elif result == "N/A":
            if evidence_text.upper() != "N/A":
                errors.append(f"{check_id}: N/A must use 'Evidence: N/A'")
            if notes.upper() in {"N/A", "NONE", "NOT APPLICABLE", ""} or len(notes) < 20:
                errors.append(f"{check_id}: N/A requires a specific reason of at least 20 characters")

    if not evidence:
        errors.append("screenshot manifest contains no evidence rows")
    for evidence_id, item in sorted(evidence.items()):
        fields = {
            "file": item.file,
            "phase": item.phase,
            "theme": item.theme,
            "surface": item.surface,
            "state": item.state,
            "size": item.size,
            "viewport": item.viewport,
            "purpose": item.purpose,
        }
        for field_name, value in fields.items():
            if not value or value.upper() in {"TODO", "N/A"}:
                errors.append(f"{evidence_id}: manifest field '{field_name}' is incomplete")
        path = resolve_evidence_path(report, item.file)
        if path.suffix.lower() not in IMAGE_SUFFIXES:
            errors.append(f"{evidence_id}: evidence file is not a supported image: {item.file}")
        elif not path.is_file():
            errors.append(f"{evidence_id}: evidence image does not exist: {path}")

    phases = {item.phase.casefold() for item in evidence.values()}
    if checks.get("SJ-SCOPE-02") and checks["SJ-SCOPE-02"].group("result") == "PASS":
        if "before" not in phases:
            errors.append("SJ-SCOPE-02 is PASS but the manifest has no Before evidence")
    if "after" not in phases:
        errors.append("manifest has no After evidence")

    themes = {item.theme.casefold() for item in evidence.values() if item.phase.casefold() == "after"}
    if checks.get("SJ-FOUND-01") and checks["SJ-FOUND-01"].group("result") == "PASS":
        if "dark" not in themes:
            errors.append("SJ-FOUND-01 is PASS but the manifest has no dark After evidence")
    if checks.get("SJ-FOUND-02") and checks["SJ-FOUND-02"].group("result") == "PASS":
        if "light" not in themes:
            errors.append("SJ-FOUND-02 is PASS but the manifest has no light After evidence")

    sizes = {item.size.casefold() for item in evidence.values() if item.phase.casefold() == "after"}
    if checks.get("SJ-RESP-01") and checks["SJ-RESP-01"].group("result") == "PASS":
        for required_size in ("normal", "narrow", "minimum"):
            if required_size not in sizes:
                errors.append(
                    f"SJ-RESP-01 is PASS but the manifest has no {required_size.title()} After evidence"
                )

    if errors:
        print("Stack Junkie compliance validation: FAIL")
        for error in errors:
            print(f"- {error}")
        return 1

    label = "independent compliance" if args.audit_level == "independent" else "self-audit"
    print(f"Stack Junkie {label} validation: PASS")
    print(f"- Checklist items: {len(expected)}")
    print(f"- Evidence images: {len(evidence)}")
    if args.audit_level == "independent":
        print(f"- Independent verifier: {metadata['Independent verifier']}")
    else:
        print("- Claim ceiling: Implemented and self-audited")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
