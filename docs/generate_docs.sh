#!/bin/bash
## Must be run from within the venv.

if [[ $1 == "clean" ]]; then
    rm -rf _build
    set "rebuild"
fi

if [[ $1 == "rebuild" ]]; then
    make --no-print-directory -C ../build doc
    rm -rf apidoc
    breathe-apidoc --generate class,interface,struct,union --members --force --output-dir apidoc ../build/xml/
fi

make html
