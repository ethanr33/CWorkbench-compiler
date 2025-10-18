
import pytest
import json
from pathlib import Path
import subprocess
import os

ROOT_DIR = Path(__file__).resolve().parents[2]
CURRENT_DIR = Path(__file__).resolve().parent

COMPILER_RUN_SCRIPT = ROOT_DIR / "run.sh"
OUTPUT_BINARY = ROOT_DIR / "tmp/out"

BASIC_TESTS_ROOT_DIR = CURRENT_DIR / "basic_tests"
ARITHMETIC_EXPR_TESTS_ROOT_DIR = CURRENT_DIR / "arithmetic_expr_tests"
LOCAL_VARIABLES_TESTS_ROOT_DIR = CURRENT_DIR / "local_variables"

BASIC_TESTS_ANSWER_SOURCE = BASIC_TESTS_ROOT_DIR / "expected.json"
ARITHMETIC_EXPR_TESTS_ANSWER_SOURCE = ARITHMETIC_EXPR_TESTS_ROOT_DIR / "expected.json"
LOCAL_VARIABLES_TESTS_ANSWER_SOURCE = LOCAL_VARIABLES_TESTS_ROOT_DIR / "expected.json"

def run_test_file(file_path):
    compiler_proc = subprocess.run([str(COMPILER_RUN_SCRIPT), file_path, "--compile-only"], capture_output=True)
    print(compiler_proc)

    compilation_exit_code = compiler_proc.returncode 
    program_exit_code = -1 # Return code of -1 for the program will represent that the program failed compilation

    # Exit code of 0 means there were no compilation errors
    if compilation_exit_code == 0:
        outputted_binary_proc = subprocess.run([str(OUTPUT_BINARY)], capture_output=True)
        program_exit_code = outputted_binary_proc.returncode

    return compilation_exit_code, program_exit_code

def run_test_suite(tests_root_dir, tests_answer_source):
    with open(tests_answer_source) as f:
        content = f.read()
        answer_key = json.loads(content)

        for file_name in answer_key:
            file_path = CURRENT_DIR.relative_to(ROOT_DIR) / tests_root_dir / file_name
            print("Running test file ", file_name)
            compiler_exit_code, program_exit_code = run_test_file(file_path)
            if not answer_key[file_name]["error"]:
                # No expected error, compilation should succeed
                assert compiler_exit_code == 0
                assert program_exit_code == answer_key[file_name]["returncode"]
            else:
                # Expected a compilation error 
                assert compiler_exit_code == 1
            print("Passed", file_name)

def test_basic_tests():
    run_test_suite(BASIC_TESTS_ROOT_DIR, BASIC_TESTS_ANSWER_SOURCE)

def test_arithmetic_expr_tests():
    run_test_suite(ARITHMETIC_EXPR_TESTS_ROOT_DIR, ARITHMETIC_EXPR_TESTS_ANSWER_SOURCE)

def test_local_variables_tests():
    run_test_suite(LOCAL_VARIABLES_TESTS_ROOT_DIR, LOCAL_VARIABLES_TESTS_ANSWER_SOURCE)


