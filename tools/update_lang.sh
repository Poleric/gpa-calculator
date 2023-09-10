#!/bin/bash -e

PACKAGE_NAME="gpa-calculator"
TEXTDOMAIN=${PACKAGE_NAME}

CURRENT_DIR=$(dirname "$0")
TOP_LEVEL_DIR=$(dirname "$CURRENT_DIR")
SOURCE_DIR="$TOP_LEVEL_DIR/src"
LOCALE_DIR="$TOP_LEVEL_DIR/i18n"
POT_PATH="${LOCALE_DIR}/${TEXTDOMAIN}.pot"

# generate sources list
echo -e "" > sources.txt
find "${SOURCE_DIR}" -type f -iname "*.c" > sources.txt

# generate pot
xgettext --keyword=_ --force-po --add-comments --from-code=UTF-8 --language=C --package-name="${PACKAGE_NAME}" --package-version=1.0 --msgid-bugs-address=noreply@example.com -o "${POT_PATH}" -f sources.txt

for locale in "${LOCALE_DIR}"/*/ ; do
    locale_name=$(basename "$locale")
    po_path="${locale}/${locale_name}.po"
    LC_MESSAGES_path="${locale}/LC_MESSAGES/"
    mo_path="${LC_MESSAGES_path}/${TEXTDOMAIN}.mo"

    # update po, generate po if dont exists
    if [ -f "${po_path}" ]; then
        msgmerge --update "${po_path}" "${POT_PATH}"
    else
        msginit -i "${POT_PATH}" -l "${locale_name}.UTF-8" --no-translator -o "${po_path}"
    fi

    # compile po file into mo
    [ -d "${LC_MESSAGES_path}" ] || mkdir "${LC_MESSAGES_path}"
    msgfmt "${po_path}" -o "${mo_path}"
done

rm "sources.txt"
