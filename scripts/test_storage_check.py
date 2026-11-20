#!/usr/bin/env python3
"""
Tests for the fix to kiwix-desktop issue #1451:
  "Kiwix reads external drive as having no available storage"

Observed failure: the old code treated a negative QStorageInfo::bytesAvailable()
result as "no space available" and threw a Storage Error. The exact reason
Qt cannot provide the value in the reported Flatpak setup has not been
confirmed from this repository.

Fix: treat an unavailable free-space value as unknown instead of as zero. Keep
the quota check when Qt provides a valid value, and keep the maximum-file-size
check whenever Qt can identify the filesystem.

These tests verify the logic of the fix by inspecting the C++ source directly.
They do NOT require compilation - they are fast static/textual checks that:
  1. Confirm the broken pattern (treating bytesAvailable==-1 as no-space) is GONE
  2. Confirm the free-space comparison ignores negative/unknown values
  3. Confirm the filesystem maximum-file-size check remains present
  4. Confirm the directory-existence and writability checks are still present
  5. Confirm the fix is narrowly scoped and does not remove any other check
"""

import os
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(REPO_ROOT, "src", "downloadmanagement.cpp")


def load():
    with open(SRC, encoding="utf-8") as f:
        return f.read()


def test_old_broken_pattern_removed(src):
    """The old code had:  if (bytesAvailable == -1 || ...)
    That line must NOT exist anymore - it caused the false 'no storage' error."""
    assert "bytesAvailable == -1" not in src, (
        "FAIL: old broken pattern 'bytesAvailable == -1' still present in "
        "downloadmanagement.cpp - the fix was not applied!"
    )
    print("PASS: old broken pattern (bytesAvailable == -1) is gone.")


def test_isvalid_isready_guard_present(src):
    """Read filesystem metrics only when storage metadata is available."""
    assert "storage.isValid() && storage.isReady()" in src, (
        "FAIL: guard 'storage.isValid() && storage.isReady()' not found in "
        "downloadmanagement.cpp - the fix is missing!"
    )
    print("PASS: isValid() && isReady() guard is present.")


def test_quota_check_inside_guard(src):
    """The free-space rejection must be guarded against unknown values."""
    # Find the guard block
    guard_start = src.find("storage.isValid() && storage.isReady()")
    assert guard_start != -1, "isValid/isReady guard not found"

    # Find the closing brace of the guard block
    brace_open = src.find("{", guard_start)
    assert brace_open != -1
    depth = 1
    pos = brace_open + 1
    while pos < len(src) and depth > 0:
        if src[pos] == "{":
            depth += 1
        elif src[pos] == "}":
            depth -= 1
        pos += 1
    guard_block = src[brace_open:pos]

    assert "bytesAvailable" in guard_block, (
        "FAIL: bytesAvailable not found inside the isValid/isReady guard block."
    )
    assert "download-storage-error-text" in guard_block, (
        "FAIL: download-storage-error-text throw not inside the guard block."
    )
    assert "bytesAvailable >= 0" in guard_block, (
        "FAIL: negative/unknown bytesAvailable values are not excluded."
    )
    print("PASS: free-space check ignores negative/unknown values.")


def test_max_file_size_check_preserved(src):
    """The filesystem maximum-file-size guard must remain available."""
    assert "if ( storage.isValid() && storage.isReady() &&\n         book.getSize() > getMaxFileSize(storage) )" in src, (
        "FAIL: maximum-file-size check is missing."
    )
    assert '"download-exceeds-max-file-size"' in src, (
        "FAIL: maximum-file-size error is missing."
    )
    print("PASS: filesystem maximum-file-size check is present.")


def test_storage_diagnostic_fields(src):
    """The diagnostic line must include the values needed to investigate #1451."""
    diagnostic_start = src.find('qInfo() << "Download storage check:"')
    assert diagnostic_start != -1, "FAIL: storage diagnostic log is missing."
    diagnostic_end = src.find(";", diagnostic_start)
    diagnostic = src[diagnostic_start:diagnostic_end]
    for field in (
        '"targetDir="',
        '"isValid="',
        '"isReady="',
        '"rootPath="',
        '"bytesAvailable="',
        '"bytesTotal="',
    ):
        assert field in diagnostic, f"FAIL: {field} missing from storage diagnostic."
    print("PASS: storage diagnostic includes all requested fields.")


def test_directory_existence_check_still_present(src):
    """The fix must NOT remove the check that the target directory exists."""
    assert "targetDirInfo.isDir()" in src, (
        "FAIL: isDir() check was removed - directory existence check is gone!"
    )
    assert "download-dir-missing" in src, (
        "FAIL: download-dir-missing error is gone - directory existence check missing!"
    )
    print("PASS: directory existence check (isDir) is still present.")


def test_writability_check_still_present(src):
    """The fix must NOT remove the writability check."""
    assert "targetDirInfo.isWritable()" in src, (
        "FAIL: isWritable() check was removed!"
    )
    assert "download-dir-not-writable" in src, (
        "FAIL: download-dir-not-writable error is gone - writability check missing!"
    )
    print("PASS: writability check (isWritable) is still present.")


def test_storage_info_constructed_before_guard(src):
    """QStorageInfo object must be constructed before the guard."""
    guard_start = src.find("storage.isValid() && storage.isReady()")
    storage_ctor = src.find("QStorageInfo storage(targetDir)")
    assert storage_ctor != -1, "QStorageInfo storage(targetDir) not found"
    assert storage_ctor < guard_start, (
        "FAIL: QStorageInfo is constructed AFTER the guard - wrong order!"
    )
    print("PASS: QStorageInfo is constructed before the isValid/isReady guard.")


def test_no_bytesavailable_outside_guard(src):
    """Read storage metrics for logging, but enforce quota inside the guard."""
    import re

    # Strip C++ line comments (//) and block comments (/* */) before counting
    def strip_comments(text):
        text = re.sub(r'//[^\n]*', '', text)
        text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
        return text

    src_no_comments = strip_comments(src)

    assert src_no_comments.count("storage.bytesAvailable()") == 1, (
        "FAIL: expected one bytesAvailable() read for the diagnostic and check."
    )

    guard_start = src_no_comments.find("storage.isValid() && storage.isReady()")
    brace_open = src_no_comments.find("{", guard_start)
    depth = 1
    pos = brace_open + 1
    while pos < len(src_no_comments) and depth > 0:
        if src_no_comments[pos] == "{":
            depth += 1
        elif src_no_comments[pos] == "}":
            depth -= 1
        pos += 1
    guard_block = src_no_comments[brace_open:pos]
    assert "bytesAvailable >= 0" in guard_block, (
        "FAIL: free-space check is not protected from an unknown value."
    )
    print("PASS: bytesAvailable is logged once and checked inside the guard.")


if __name__ == "__main__":
    src = load()
    tests = [
        test_old_broken_pattern_removed,
        test_isvalid_isready_guard_present,
        test_quota_check_inside_guard,
        test_max_file_size_check_preserved,
        test_storage_diagnostic_fields,
        test_directory_existence_check_still_present,
        test_writability_check_still_present,
        test_storage_info_constructed_before_guard,
        test_no_bytesavailable_outside_guard,
    ]
    failed = 0
    for t in tests:
        try:
            t(src)
        except AssertionError as e:
            print(e)
            failed += 1
    if failed:
        print(f"\n{failed} test(s) FAILED.")
        sys.exit(1)
    print("\nAll storage-check tests PASSED.")
