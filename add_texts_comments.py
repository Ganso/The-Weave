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

def find_dialogs_and_choices(c_file, texts_h_file, texts_c_file):
    print(f"\nProcessing file: {c_file}")
    print(f"Using texts.h: {texts_h_file}")
    print(f"Using texts.c: {texts_c_file}")
    # Load dialog and choice IDs from texts.h
    dialogs = {}
    choices = {}
    # Add SYSTEM_DIALOG explicitly since it's always 0
    dialogs[0] = "SYSTEM_DIALOG"
    
    with open(texts_h_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        for line in lines:
            dialog_match = re.match(r'^\s*#define\s+(\w+)_DIALOG(\d+)\s+(\d+)', line)
            choice_match = re.match(r'^\s*#define\s+(\w+)_CHOICE(\d+)\s+(\d+)', line)
            if dialog_match:
                act_name = dialog_match.group(1)
                dialog_num = dialog_match.group(2)
                dialog_id = int(dialog_match.group(3))
                dialogs[dialog_id] = f"{act_name}_DIALOG{dialog_num}"
                print(f"Found dialog ID mapping: {dialog_id} -> {dialogs[dialog_id]}")
            elif choice_match:
                act_name = choice_match.group(1)
                choice_num = choice_match.group(2)
                choice_id = int(choice_match.group(3))
                choices[choice_id] = f"{act_name}_CHOICE{choice_num}"
                print(f"Found choice ID mapping: {choice_id} -> {choices[choice_id]}")

    # Load dialog and choice texts from texts.c
    dialog_texts = {}
    choice_texts = {}
    
    with open(texts_c_file, 'r', encoding='utf-8') as f:
        content = f.read()
        
        # If this is texts.c, add index comments to each dialog item
        if c_file.endswith('texts.c'):
            modified_content = content
            
            print("\nLooking for dialog blocks in texts.c...")
            # Process dialog blocks
            blocks = []
            current_pos = 0
            
            # Find all dialog block starts
            print("Searching for dialog array definitions...")
            for match in re.finditer(r'const\s+DialogItem\s+(\w+)_dialog\d*\[\]\s*=\s*{', content):
                block_name = match.group(1)
                block_start = match.start()
                
                # Find the matching end of this block
                brace_count = 1
                pos = match.end()
                while brace_count > 0 and pos < len(content):
                    if content[pos] == '{':
                        brace_count += 1
                    elif content[pos] == '}':
                        brace_count -= 1
                    pos += 1
                
                if brace_count == 0:
                    # Found complete block
                    block_content = content[block_start:pos]
                    blocks.append((block_start, pos, block_content))
                    print(f"\nFound dialog block:")
                    print(f"Start: {block_start}, End: {pos}")
                    print(f"Content preview: {block_content[:200]}...")
            
            print(f"\nFound {len(blocks)} dialog blocks total")
            print("\nProcessing blocks to add/update index comments...")
            # Process each block independently
            modified_content = content
            offset = 0
            
            for block_start, block_end, block_content in blocks:
                # Extract block comment if any
                block_lines = block_content.split('\n')
                block_header = block_lines[0]
                block_comment = ""
                if '//' in block_header:
                    block_comment = '//' + block_header.split('//')[1]
                
                # Process items and preserve block structure
                new_block_lines = [block_header]
                current_item = 0
                
                # Buffer para acumular líneas de un mismo item
                item_lines = []
                in_item = False
                
                for line in block_lines[1:]:
                    # Si es una línea vacía o terminador
                    if not line.strip() or "NULL" in line:
                        # Si estábamos en un item, procesarlo
                        if in_item:
                            # Unir las líneas del item
                            full_item = ''.join(item_lines)
                            # Añadir el comentario al primer FACE_
                            if 'FACE_' in full_item:
                                first_line = item_lines[0]
                                # Remover comentario existente si hay
                                first_line = re.sub(r'\s*//.*$', '', first_line)
                                # Añadir nuevo comentario manteniendo el salto de línea original
                                if first_line.endswith('\n'):
                                    item_lines[0] = f"{first_line.rstrip()} // {current_item}\n"
                                else:
                                    item_lines[0] = f"{first_line.rstrip()} // {current_item}"
                                current_item += 1
                            # Añadir todas las líneas del item
                            new_block_lines.extend(item_lines)
                            # Resetear buffer
                            item_lines = []
                            in_item = False
                        new_block_lines.append(line)
                        continue
                    
                    # Si la línea comienza un nuevo item de diálogo
                    if '{FACE_' in line or '{0,' in line:
                        # Si estábamos en un item previo, procesarlo
                        if in_item:
                            # Unir las líneas del item
                            full_item = ''.join(item_lines)
                            # Añadir el comentario al primer FACE_
                            if 'FACE_' in full_item:
                                first_line = item_lines[0]
                                # Remover comentario existente si hay
                                first_line = re.sub(r'\s*//.*$', '', first_line)
                                # Añadir nuevo comentario
                                if first_line.endswith('\n'):
                                    item_lines[0] = f"{first_line.rstrip()} // {current_item}\n"
                                else:
                                    item_lines[0] = f"{first_line.rstrip()} // {current_item}"
                                current_item += 1
                            # Añadir todas las líneas del item
                            new_block_lines.extend(item_lines)
                            # Resetear buffer
                            item_lines = []
                        # Comenzar nuevo item
                        in_item = True
                        item_lines = [line]
                    # Si estamos dentro de un item, acumular líneas
                    elif in_item:
                        item_lines.append(line)
                    # Si no es parte de un item, añadir directamente
                    else:
                        new_block_lines.append(line)
                
                # Procesar último item si quedó alguno
                if in_item:
                    full_item = ''.join(item_lines)
                    if 'FACE_' in full_item:
                        first_line = item_lines[0]
                        first_line = re.sub(r'\s*//.*$', '', first_line)
                        item_lines[0] = f"{first_line.rstrip()} // {current_item}\n"
                    new_block_lines.extend(item_lines)
                
                # Replace the block in the content
                new_block_content = '\n'.join(new_block_lines)
                modified_content = modified_content[:block_start + offset] + new_block_content + modified_content[block_end + offset:]
                offset += len(new_block_content) - (block_end - block_start)
            
            # Write back the modified content
            with open(c_file, 'w', encoding='utf-8', newline='') as f:
                f.write(modified_content)
            print("Successfully updated dialog item numbering in texts.c")
            
        print("\nSearching for dialog and choice blocks in texts.c...")
        
        # Find all dialog array definitions
        dialog_blocks = list(re.finditer(r'const\s+DialogItem\s+(\w+)_dialog(\d+)?\[\]\s*=\s*{(.*?)};', content, re.DOTALL))
        print(f"Found {len(dialog_blocks)} dialog blocks")
        
        # Find all choice array definitions
        choice_blocks = list(re.finditer(r'const\s+ChoiceItem\s+(\w+)_choice(\d+)\[\]\s*=\s*{(.*?)};', content, re.DOTALL))
        print(f"Found {len(choice_blocks)} choice blocks")
        
        # Process dialog blocks
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

        # Process choice blocks
        for block in choice_blocks:
            act_name = block.group(1)
            choice_num = block.group(2)
            
            # Find choice ID from the mapping
            choice_id = None
            for cid, cname in choices.items():
                if cname.lower() == f"{act_name}_choice{choice_num}".lower():
                    choice_id = cid
                    break
            
            if choice_id is not None:
                block_content = block.group(3)
                print(f"\nProcessing {act_name}_choice{choice_num} (ID: {choice_id}):")
                print(f"Block content:\n{block_content}")  # Debug: Show full block content
                
                # Process each choice block
                choice_texts[choice_id] = []
                
                # First find all choice items (excluding terminator)
                items = []
                brace_count = 0
                start_pos = None
                
                # Manually parse the block to handle nested braces correctly
                for i, char in enumerate(block_content):
                    if char == '{':
                        if brace_count == 0:
                            start_pos = i
                        brace_count += 1
                    elif char == '}':
                        brace_count -= 1
                        if brace_count == 0 and start_pos is not None:
                            item = block_content[start_pos:i+1]
                            if "FACE_" in item and "NULL" not in item:
                                items.append(item)
                            start_pos = None
                
                print(f"\nFound {len(items)} non-NULL choice items")
                for item in items:
                    print(f"Found item: {item}")
                
                # Process each choice item
                for i, item in enumerate(items):
                    print(f"\nProcessing choice item {i}:")
                    print(f"Raw item: {item}")
                    
                    try:
                        # Extract number of options - more precise pattern
                        num_match = re.search(r'FACE_[^,]+,\s*[^,]+,\s*[^,]+,\s*(\d+),', item)
                        if not num_match:
                            print("Could not find number of options")
                            continue
                        num_options = int(num_match.group(1))
                        print(f"Number of options: {num_options}")
                        
                        # Extract the Spanish and English arrays - handle nested structure
                        arrays_match = re.search(r'{{\s*{([^}]*?)}\s*,\s*{\s*([^}]*?)}\s*}}', item, re.DOTALL)
                        if not arrays_match:
                            print("Could not find text arrays")
                            continue
                        
                        # Extract quoted strings from each array
                        es_text = arrays_match.group(1).strip()
                        en_text = arrays_match.group(2).strip()
                        print(f"Spanish array: {es_text}")
                        print(f"English array: {en_text}")
                        
                        # Find all quoted strings in each array
                        es_matches = list(re.finditer(r'"([^"]+)"', es_text))
                        en_matches = list(re.finditer(r'"([^"]+)"', en_text))
                        
                        # Take only the required number of options
                        if len(es_matches) >= num_options and len(en_matches) >= num_options:
                            es_options = [m.group(1).strip() for m in es_matches[:num_options]]
                            en_options = [m.group(1).strip() for m in en_matches[:num_options]]
                        else:
                            print(f"Not enough options found. Expected {num_options}, got ES:{len(es_matches)}, EN:{len(en_matches)}")
                            continue
                        
                        print(f"Spanish options: {es_options}")
                        print(f"English options: {en_options}")
                        
                        # Add each pair of options
                        for es_text, en_text in zip(es_options, en_options):
                            choice_texts[choice_id].append({
                                "es": es_text,
                                "en": en_text
                            })
                            print(f"Added option: ES='{es_text}' EN='{en_text}'")
                            
                    except Exception as e:
                        print(f"Error processing choice item: {e}")
                
                if choice_texts[choice_id]:
                    print(f"Found {len(choice_texts[choice_id])} options for choice {choice_id}")

    # Process the C file and add/update comments for talk_dialog and choice_dialog calls
    if not c_file.endswith('texts.c'):  # Skip if this is texts.c since we already processed it
        modified = False
        with open(c_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        with open(c_file, 'w', encoding='utf-8', newline='') as f:
            for line in lines:
                # Check if line has a talk_dialog call
                talk_match = re.search(r'talk_dialog\s*\(\s*&dialogs\[(\w+)_DIALOG(?:\d+)?\]\[(\d+)\]\s*\);(.*?)$', line.rstrip())
                # Check if line has a choice_dialog call
                choice_match = re.search(r'choice_dialog\s*\(\s*&choices\[(\w+)_CHOICE(\d+)\]\[0\]\s*\);(.*?)$', line.rstrip())

                # *****************************************************
                # * DON'T PROCESS CHOICES - IT'S NOT WORKING PROPERLY *
                # *****************************************************
                if choice_match:
                    choice_match = False
                    print("DISCARDING DETECTED CHOICE")
                
                if talk_match:
                    act_name = talk_match.group(1)
                    dialog_index = int(talk_match.group(2))
                    existing_comment = talk_match.group(3).strip()
                    
                    # Find the dialog ID
                    dialog_id = None
                    if act_name == "SYSTEM":
                        dialog_id = 0
                    else:
                        # Extract the dialog number from the full match
                        dialog_num_match = re.search(r'_DIALOG(\d+)', talk_match.group(0))
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
                            line = line[:talk_match.start(3)].rstrip()
                        # Add the new comment
                        comment = f' // (ES) "{texts["es"]}" - (EN) "{texts["en"]}"'
                        line = line.rstrip() + comment + '\n'
                        modified = True
                
                elif choice_match:
                    act_name = choice_match.group(1)
                    choice_num = choice_match.group(2)
                    existing_comment = choice_match.group(3).strip()
                    
                    # Find the choice ID
                    choice_id = None
                    choice_name = f"{act_name}_CHOICE{choice_num}"
                    for cid, cname in choices.items():
                        if cname == choice_name:
                            choice_id = cid
                            break
                    
                    print(f"Found choice_dialog call: {choice_name} (ID: {choice_id})")

                    if choice_id is not None and choice_id in choice_texts:
                        # If there's an existing comment, replace it
                        if re.search(r'//.*\(ES\).*\(EN\)', existing_comment):
                            line = line[:choice_match.start(3)].rstrip()
                        # Add the new comment with all options in one line
                        comment = ' // Options: '
                        options = []
                        for i, texts in enumerate(choice_texts[choice_id]):
                            options.append(f'{i+1}. (ES) "{texts["es"]}" - (EN) "{texts["en"]}"')
                        comment += ', '.join(options)
                        line = line.rstrip() + comment + '\n'
                        modified = True
                
                f.write(line)
    
        if modified:
            print(f"Successfully updated comments in {c_file}")
        else:
            print(f"No comments were modified in {c_file}")

def process_all_c_files():
    texts_h_file = "src/texts.h"
    texts_c_file = "src/texts.c"
    
    # Get all .c files in src directory
    for file in os.listdir("src"):
        if file.endswith(".c"):
            c_file = os.path.join("src", file)
            print(f"\nProcessing {c_file}...")
            find_dialogs_and_choices(c_file, texts_h_file, texts_c_file)

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

        find_dialogs_and_choices(c_file, texts_h_file, texts_c_file)

if __name__ == "__main__":
    main()
