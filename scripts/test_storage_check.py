#!/usr/bin/env python3
"""
Tests for the fix to kiwix-desktop issue #1451:
  "Kiwix reads external drive as having no available storage"

Root cause: QStorageInfo::bytesAvailable() returns -1 inside a Flatpak sandbox
for external/removable drives whose mount point is not visible inside the
sandbox's mount namespace. The old code treated -1 as "no space available" and
threw a spurious Storage Error.

Fix: skip the quota check when QStorageInfo is not valid or not ready (i.e.
when storage information is unavailable). We still verify the directory exists
and is writable, so a real permissions error is caught correctly.

These tests verify the logic of the fix by inspecting the C++ source directly.
They do NOT require compilation - they are fast static/textual checks that:
  1. Confirm the broken pattern (treating bytesAvailable==-1 as no-space) is GONE
  2. Confirm the fixed pattern (guard with isValid() && isReady()) is PRESENT
  3. Confirm the max-file-size check is also behind the same guard
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
    """The fix must guard the quota check with storage.isValid() && storage.isReady()."""
    assert "storage.isValid() && storage.isReady()" in src, (
        "FAIL: guard 'storage.isValid() && storage.isReady()' not found in "
        "downloadmanagement.cpp - the fix is missing!"
    )
    print("PASS: isValid() && isReady() guard is present.")


def test_quota_check_inside_guard(src):
    """book.getSize() > ... bytesAvailable must only appear inside the isValid guard."""
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
    assert "download-exceeds-max-file-size" in guard_block, (
        "FAIL: download-exceeds-max-file-size check not inside the guard block."
    )
    print("PASS: quota check and max-file-size check are inside the guard block.")


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
    """Make sure bytesAvailable does not appear in CODE outside the guard block.
    Comments are excluded from this check."""
    import re

    # Strip C++ line comments (//) and block comments (/* */) before counting
    def strip_comments(text):
        text = re.sub(r'//[^\n]*', '', text)
        text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
        return text

    src_no_comments = strip_comments(src)

    count_total = src_no_comments.count("bytesAvailable")

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
    count_inside_guard = guard_block.count("bytesAvailable")

    assert count_total == count_inside_guard, (
        f"FAIL: bytesAvailable appears {count_total} times in code (excluding "
        f"comments) but only {count_inside_guard} times inside the guard block "
        "- there may be a stray usage outside the guard!"
    )
    print(f"PASS: bytesAvailable appears exactly {count_total} time(s) in code, all inside the guard.")


if __name__ == "__main__":
    src = load()
    tests = [
        test_old_broken_pattern_removed,
        test_isvalid_isready_guard_present,
        test_quota_check_inside_guard,
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
