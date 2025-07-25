#!/bin/bash

set -e

# Test 1: Checking the output of letters without spaces
test_letters() {
    echo "Running test_letters..."
    echo {A..Z} | tr -d ' ' > /tmp/eve_pipe
    echo {a..z} | tr -d ' ' > /tmp/eve_pipe
}

# Test 2: Verification of ANSI codes
test_ansi() {
    echo "Running test_ansi..."
    echo -e "\e[32mHello, ANSI!\e[0m" | cat -v > /tmp/eve_pipe
}

# Test 3: Number output
test_numbers() {
    echo "Running test_numbers..."
    seq 1 10 > /tmp/eve_pipe
    cat /tmp/eve_pipe
}

if [[ -n "$1" ]]; then
    case "$1" in
        letters) test_letters ;;
        ansi) test_ansi ;;
        numbers) test_numbers ;;
        all) test_letters; test_ansi; test_numbers ;;
        *) echo "Usage: $0 {letters|ansi|numbers|all}" ;;
    esac
else
    echo "Usage: $0 {letters|ansi|numbers|all}"
fi
