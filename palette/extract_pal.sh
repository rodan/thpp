#!/bin/bash

# script that extracts the palette parameters from my Scalable Vector Graphics images

in_file=$1

grep -q '<svg' "${in_file}" || {
    echo "file contents do not look like valid SVG, exiting"
    exit 1
}

name=$(basename "${in_file}" | sed 's|\.svg||')
out=$(grep -E '(stop-color)|(offset)' "${in_file}" | sed 's| ||g' | sed '/style/ { N; s/\n// }')
stop_cnt=$(echo "$out" | wc -l)
rm -f /tmp/stop_offset
rm -f /tmp/rgb

grep -E '(stop-color)|(offset)' "${in_file}" | sed 's| ||g' | sed '/style/ { N; s/\n// }' | while read -r line; do
    foffset=$(echo "${line}" | cut -d'"' -f4)
    moff=$(echo "scale=0; 65535 * ${foffset} + 0.5" | bc)
    offset=$(printf "%f\n" "${moff}" | cut -d'.' -f1)
    echo -n " ${offset}," >> /tmp/stop_offset
    rgb_col=$(echo "${line}" | sed 's|.*#\([0-9a-f]*\).*|\1|' | sed 's|\([0-9a-f][0-9a-f]\)\([0-9a-f][0-9a-f]\)\([0-9a-f][0-9a-f]\)|0x\1, 0x\2, 0x\3|')
    echo " ${rgb_col}," >> /tmp/rgb
done

stop_offset=$(cat /tmp/stop_offset | sed 's|,$||')
rgb=$(cat /tmp/rgb)

echo '{'
echo "    \"${name}\","
echo "    ${stop_cnt},"
echo "    { ${stop_offset} },"
echo "    { ${rgb} }"
echo '},'

