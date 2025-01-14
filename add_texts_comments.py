import os
import re
import sys

def show_help():
    print("""
Add dialog text comments to C source files

Usage:
    python add_texts_comments.py <file>
    python add_texts_comments.py *

Arguments:
    file    Path to a C source file to process (e.g., act_1.c)
            Will be looked for in the src/ directory
    *       Process all .c files in src/ directory (except texts.c)

Description:
    This script adds comments to talk_dialog() calls showing the Spanish and English
    text for each dialog. It reads the dialog texts from texts.c and texts.h.

    For each talk_dialog() call, it adds a comment like:
    // (ES) "Spanish text" - (EN) "English text"

    If a similar comment already exists, it will be replaced with the current text.

Examples:
    python add_texts_comments.py act_1.c
    python add_texts_comments.py *
""")

def find_dialogs(c_file, texts_h_file, texts_c_file):
    # Load dialog IDs from texts.h
    dialogs = {}
    # Add SYSTEM_DIALOG explicitly since it's always 0
    dialogs[0] = "SYSTEM_DIALOG"
    
    with open(texts_h_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        for line in lines:
            match = re.match(r'^\s*#define\s+(\w+)_DIALOG(\d+)\s+(\d+)', line)
            if match:
                act_name = match.group(1)
                dialog_num = match.group(2)
                dialog_id = int(match.group(3))
                dialogs[dialog_id] = f"{act_name}_DIALOG{dialog_num}"
                print(f"Found dialog ID mapping: {dialog_id} -> {dialogs[dialog_id]}")

    # Load dialog texts from texts.c
    dialog_texts = {}
    
    with open(texts_c_file, 'r', encoding='utf-8') as f:
        content = f.read()
        print("\nSearching for dialog blocks in texts.c...")
        
        # Find all dialog array definitions
        # Match everything between the opening { after the array declaration and the final };
        dialog_blocks = list(re.finditer(r'const\s+DialogItem\s+(\w+)_dialog(\d+)?\[\]\s*=\s*{(.*?)};', content, re.DOTALL))
        print(f"Found {len(dialog_blocks)} dialog blocks")
        
        for block in dialog_blocks:
            act_name = block.group(1)
            dialog_num = block.group(2)  # This will be None for system_dialog
            
            # Find dialog ID from the mapping
            dialog_id = None
            if dialog_num is None and act_name.lower() == "system":
                dialog_id = 0  # SYSTEM_DIALOG is always 0
            else:
                for did, dname in dialogs.items():
                    if dname.lower() == f"{act_name}_dialog{dialog_num}".lower():
                        dialog_id = did
                        break
            
            if dialog_id is not None:
                block_content = block.group(3)
                print(f"\nProcessing {act_name}_dialog{dialog_num or ''} (ID: {dialog_id}):")
                
                # Split block into individual dialog items
                # Match each complete dialog item structure
                items = list(re.finditer(r'{[^{]+?{\s*"([^"]+)",\s*"([^"]+)"\s*}\s*}', block_content))
                print(f"Found {len(items)} dialog items")
                
                dialog_texts[dialog_id] = []
                for item in items:
                    if "NULL" not in item.group(0):  # Skip terminator entries
                        es_text = item.group(1).replace("|", " ")
                        en_text = item.group(2).replace("|", " ")
                        dialog_texts[dialog_id].append({
                            "es": es_text,
                            "en": en_text
                        })
                        print(f"  Found text: ES='{es_text}' EN='{en_text}'")
                print(f"Found {len(dialog_texts[dialog_id])} texts for dialog {dialog_id}")

    # Process the C file and add/update comments for talk_dialog calls
    modified = False
    with open(c_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    with open(c_file, 'w', encoding='utf-8', newline='') as f:
        for line in lines:
            # Check if line has a talk_dialog call
            match = re.search(r'talk_dialog\s*\(\s*&dialogs\[(\w+)_DIALOG(?:\d+)?\]\[(\d+)\]\s*\);(.*?)$', line.rstrip())
            if match:
                act_name = match.group(1)
                dialog_index = int(match.group(2))
                existing_comment = match.group(3).strip()
                
                # Find the dialog ID
                dialog_id = None
                if act_name == "SYSTEM":
                    dialog_id = 0
                else:
                    # Extract the dialog number from the full match
                    dialog_num_match = re.search(r'_DIALOG(\d+)', match.group(0))
                    if dialog_num_match:
                        dialog_name = f"{act_name}_DIALOG{dialog_num_match.group(1)}"
                        for did, dname in dialogs.items():
                            if dname == dialog_name:
                                dialog_id = did
                                break
                
                print(f"Found talk_dialog call: {act_name}_DIALOG[{dialog_index}] (ID: {dialog_id})")

                if dialog_id is not None and dialog_id in dialog_texts and dialog_index < len(dialog_texts[dialog_id]):
                    texts = dialog_texts[dialog_id][dialog_index]
                    # If there's an existing ES/EN comment, replace it
                    if re.search(r'//.*\(ES\).*\(EN\)', existing_comment):
                        line = line[:match.start(3)].rstrip()
                    # Add the new comment
                    comment = f' // (ES) "{texts["es"]}" - (EN) "{texts["en"]}"'
                    line = line.rstrip() + comment + '\n'
                    modified = True
            f.write(line)
    
    if modified:
        print(f"Successfully updated dialog comments in {c_file}")
    else:
        print(f"No dialog comments were modified in {c_file}")

def process_all_c_files():
    texts_h_file = "src/texts.h"
    texts_c_file = "src/texts.c"
    
    # Get all .c files in src directory except texts.c
    for file in os.listdir("src"):
        if file.endswith(".c") and file != "texts.c":
            c_file = os.path.join("src", file)
            print(f"\nProcessing {c_file}...")
            find_dialogs(c_file, texts_h_file, texts_c_file)

def main():
    # Show help if no arguments provided
    if len(sys.argv) < 2:
        show_help()
        return

    texts_h_file = "src/texts.h"
    texts_c_file = "src/texts.c"

    # Process based on argument
    if sys.argv[1] == "*":
        process_all_c_files()
    else:
        c_file = sys.argv[1]
        # Add src/ prefix if not provided
        if not c_file.startswith("src/"):
            c_file = f"src/{c_file}"

        if not os.path.exists(c_file):
            print(f"Error: File {c_file} not found")
            return

        find_dialogs(c_file, texts_h_file, texts_c_file)

if __name__ == "__main__":
    main()
