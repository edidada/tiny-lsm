#! /usr/bin/sh

# Check if at least one argument is provided
if [ -z "$1" ]; then
  echo "Error: No commit message provided."
  exit 1
fi

rm -rf book
rm -rf ../toni-lsm-web/book

mdbook build

mv book/ ../toni-lsm-web/
cd ../toni-lsm-web
git add -A
git commit -m "$1"
git push origin gh-pages