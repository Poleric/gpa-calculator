#  To tired to learn batch
import os

# capital names are absolute path
PACKAGE_NAME="gpa-calculator"
TEXTDOMAIN=PACKAGE_NAME  # can be changed

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TOP_LEVEL_DIR = CURRENT_DIR + "\.."
SOURCE_DIR = TOP_LEVEL_DIR + "\src"
LOCALE_DIR = TOP_LEVEL_DIR + "\i18n"

POT_PATH = LOCALE_DIR + f"\{TEXTDOMAIN}.pot"

# generate pot
os.chdir(SOURCE_DIR)
source_files = [f for f in os.listdir() if f.endswith(".c")]
sources = "\" \"".join(source_files)
os.system(f"xgettext --keyword=_ --force-po --add-comments --from-code=UTF-8 --language=C --package-name=\"{PACKAGE_NAME}\" --package-version=1.0 --msgid-bugs-address=noreply@example.com -o \"{POT_PATH}\" \"{sources}\"")

os.chdir(LOCALE_DIR)
for locale_dir in (dir for dir in os.listdir() if os.path.isdir(dir)):
    LC_MESSAGES_path = f".\{locale_dir}\LC_MESSAGES"
    po_path = f".\{locale_dir}\{locale_dir}.po"
    mo_path = f"{LC_MESSAGES_path}\{TEXTDOMAIN}.mo"

    # update po, generate if not exists
    if os.path.exists(po_path):
        os.system(f"msgmerge --update \"{po_path}\" \"{POT_PATH}\"")
    else:
        os.system(f"msginit -i \"{POT_PATH}\" -l \"{locale_dir}.UTF-8\" --no-translator -o \"{po_path}\"")
    
    # compile po file to mo
    os.makedirs(LC_MESSAGES_path, exist_ok=True)
    os.system(f"msgfmt \"{po_path}\" -o \"{mo_path}\"")
