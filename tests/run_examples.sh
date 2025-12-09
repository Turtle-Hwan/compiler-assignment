#!/usr/bin/env sh
# Run every Mini-JS example and compare its output with the recorded expectation.

set -eu

QUIET_FLAG="${QUIET_FLAG:--q}"
DIFF_FLAGS="${DIFF_FLAGS:---strip-trailing-cr}"

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "${SCRIPT_DIR}/.." && pwd)"
BINARY="${1:-${PROJECT_ROOT}/minijs}"
EXAMPLES_DIR="${2:-${PROJECT_ROOT}/examples}"
EXPECTED_DIR="${3:-${EXAMPLES_DIR}/expected}"

if [ ! -x "${BINARY}" ]; then
    echo "error: binary not found or not executable: ${BINARY}" >&2
    exit 2
fi

if [ ! -d "${EXAMPLES_DIR}" ]; then
    echo "error: examples directory missing: ${EXAMPLES_DIR}" >&2
    exit 2
fi

if [ ! -d "${EXPECTED_DIR}" ]; then
    echo "error: expected output directory missing: ${EXPECTED_DIR}" >&2
    exit 2
fi

STATUS=0
TMP_OUT=""
TMP_DIFF=""

clear_tmps() {
    if [ -n "${TMP_OUT}" ] && [ -f "${TMP_OUT}" ]; then
        rm -f "${TMP_OUT}"
    fi
    if [ -n "${TMP_DIFF}" ] && [ -f "${TMP_DIFF}" ]; then
        rm -f "${TMP_DIFF}"
    fi
    TMP_OUT=""
    TMP_DIFF=""
}

cleanup() {
    clear_tmps
}

trap cleanup EXIT

print_actual_output() {
    if [ -n "${TMP_OUT}" ] && [ -s "${TMP_OUT}" ]; then
        awk 'NR==1 { printf "    output: %s\n", $0; next } { printf "            %s\n", $0 }' "${TMP_OUT}"
    else
        echo "    output: (empty)"
    fi
}

for JS_FILE in "${EXAMPLES_DIR}"/*.js; do
    [ -e "${JS_FILE}" ] || {
        echo "error: no .js files in ${EXAMPLES_DIR}" >&2
        exit 2
    }

    BASENAME="$(basename "${JS_FILE}" .js)"
    EXPECTED_FILE="${EXPECTED_DIR}/${BASENAME}.txt"

    echo "-> ${BASENAME}"

    if [ ! -f "${EXPECTED_FILE}" ]; then
        echo "   FAIL" && echo "    - missing expected output: ${EXPECTED_FILE}"
        STATUS=1
        continue
    fi

    TMP_OUT="$(mktemp)"
    TMP_DIFF="$(mktemp)"

    if ! "${BINARY}" "${QUIET_FLAG}" -e "${JS_FILE}" >"${TMP_OUT}" 2>"${TMP_DIFF}"; then
        echo "   FAIL"
        echo "    - interpreter exited with non-zero status"
        if [ -s "${TMP_DIFF}" ]; then
            sed 's/^/    stderr: /' "${TMP_DIFF}"
        else
            echo "    stderr: (empty)"
        fi
        print_actual_output
        STATUS=1
        clear_tmps
        continue
    fi

    if ! diff -u ${DIFF_FLAGS} "${EXPECTED_FILE}" "${TMP_OUT}" >"${TMP_DIFF}"; then
        echo "   FAIL"
        sed 's/^/    /' "${TMP_DIFF}"
        print_actual_output
        STATUS=1
    else
        echo "   PASS"
        print_actual_output
    fi

    clear_tmps

done

if [ ${STATUS} -eq 0 ]; then
    printf "\nAll example tests passed.\n"
else
    printf "\nSome tests failed.\n" >&2
fi

exit ${STATUS}
