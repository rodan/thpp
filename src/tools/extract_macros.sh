#!/bin/bash

extract_defs()
{
    grep '^#define' "${1}" | tr -s '[:space:]' | sed 's|\(.*\)\s//.*|\1|;s|\s*$||;s|#define |-D|g;s| |=|g' | grep -Ev '(_H_)|(=.*=)' | xargs
}

while (( "$#" )); do
    if [ "$1" = "-t" ]; then
        target="${2}"
        shift; shift;
    elif [ "$1" = "-f" ]; then
        file_list="${file_list} ${2}"
        shift; shift;
    else
        file_list="${file_list} ${1}"
        shift;
    fi
done

for file in ${file_list}; do
    targeted_file="${file//.h/}_${target}.h"
    if [ -e "${targeted_file}" ]; then
        EXTR_STR="$(extract_defs "${targeted_file}") ${EXTR_STR}"
    elif [ -e "${file}" ]; then
        EXTR_STR="$(extract_defs "${file}") ${EXTR_STR}"
    fi
done

echo "${EXTR_STR}"
