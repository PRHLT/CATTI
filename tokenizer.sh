sed -e 's/\([,\.;]\) / \1 /g' -e 's/\([,\.;]\)$/ \1/g' -e 's/\([,\.;]\)_/ \1 _/g' -e 's/\([_:!?\x22\x27\(\)]\)/ \1 /g' -e 's/\]/ \] /g' -e 's/\[/ \[ /g' -e 's/[ ]* / /g' $1

