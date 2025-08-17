#!/bin/bash

find . -name "*.cpp" -o -name "*.h" | while read file; do
  echo "=== $file ===" >> output.txt
  cat "$file" >> output.txt
  echo "" >> output.txt
done

echo "Done!"
