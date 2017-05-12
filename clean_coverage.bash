#! /bin/bash
find $1 -name "*.gcda" -print0 | xargs -0 rm