import os, re, sys  


def show_help():  
    """
    Print usage instructions for the script.
    """
    print("""  
Add dialog text comments to C source files  

Usage:  
    python add_texts_comments.py <file>  
    python add_texts_comments.py *  

This script reads the dialog texts from texts.c and texts_generated.c and adds
comments to talk_dialog(), talk_cluster() and choice_dialog() calls.
""")  


def read_mappings(texts_h_file):  
    """
    Parse texts.h to build mappings from dialog/choice IDs to their symbolic names.
    Returns two dictionaries: dialogs and choices.
    """
    dialogs = {0: "SYSTEM_DIALOG"}  # Default value  
    choices = {}  
    with open(texts_h_file, 'r', encoding='utf-8') as f:  
        for line in f:  
            # Match dialog and choice #define lines
            dmatch = re.match(r'^\s*#define\s+(\w+)_DIALOG(\d+)\s+(\d+)', line)  
            cmatch = re.match(r'^\s*#define\s+(\w+)_CHOICE(\d+)\s+(\d+)', line)  
            if dmatch:  
                act = dmatch.group(1)  
                num = dmatch.group(2)  
                id_ = int(dmatch.group(3))  
                dialogs[id_] = f"{act}_DIALOG{num}"  
            elif cmatch:  
                act = cmatch.group(1)  
                num = cmatch.group(2)  
                id_ = int(cmatch.group(3))  
                choices[id_] = f"{act}_CHOICE{num}"  
    return dialogs, choices  


def extract_items(block_content):  
    """
    Extracts individual struct items from a C array block (between braces).
    Ignores items containing 'NULL' (used as terminators).
    """
    items = []  
    brace_count = 0  
    start = None  
    for i, ch in enumerate(block_content):  
        if ch == '{':  
            if brace_count == 0:  
                start = i  
            brace_count += 1  
        elif ch == '}':  
            brace_count -= 1  
            if brace_count == 0 and start is not None:  
                item = block_content[start:i+1]  
                if "NULL" not in item:  # Ignore terminator  
                    items.append(item)  
                start = None  
    return items  


def parse_dialog_item(item):  
    """
    Parse a DialogItem struct to extract Spanish and English text.
    Returns a dict with 'es' and 'en' keys, or None if not matched.
    """
    m = re.search(r'\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}', item, re.DOTALL)  
    if m:  
        return {"es": m.group(1).strip(), "en": m.group(2).strip()}  
    return None  


def parse_choice_item(item):  
    """
    Parse a ChoiceItem struct to extract the options in Spanish and English.
    Returns a list of dicts with 'es' and 'en' keys for each option.
    """
    # Find the number of options
    count_match = re.search(r'FACE_[^,]+,\s*[^,]+,\s*[^,]+,\s*(\d+),', item)  
    if not count_match:  
        return []  
    num_options = int(count_match.group(1))  
    # Extract the arrays of options for ES and EN
    arrays_match = re.search(r'\{\s*\{(.*?)\}\s*,\s*\{(.*?)\}\s*\}', item, re.DOTALL)  
    if not arrays_match:  
        return []  
    es_text = arrays_match.group(1)  
    en_text = arrays_match.group(2)  
    es_options = re.findall(r'"([^"]*?)"', es_text)[:num_options]  
    en_options = re.findall(r'"([^"]*?)"', en_text)[:num_options]  
    return [{"es": es.strip(), "en": en.strip()} for es, en in zip(es_options, en_options)]  


def parse_texts(files):
    """
    Parse the provided C files to extract dialog, cluster and choice texts.
    Returns three dictionaries keyed by their array name in uppercase.
    """
    dialog_texts = {}
    choice_texts = {}
    cluster_texts = {}

    if isinstance(files, str):
        files = [files]

    for path in files:
        with open(path, 'r', encoding='utf-8') as f:
            content = f.read()

        # Dialog and cluster blocks
        dlg_blocks = re.finditer(r'(?:static\s+)?const\s+DialogItem\s+(\w+)\[\]\s*=\s*{(.*?)};', content, re.DOTALL)
        for block in dlg_blocks:
            name = block.group(1)
            items = extract_items(block.group(2))
            texts = []
            for item in items:
                res = parse_dialog_item(item)
                if res:
                    texts.append(res)
            if name.startswith('cluster_'):
                cluster_texts[name.upper()] = texts
            else:
                dialog_texts[name.upper()] = texts

        # Choice blocks (only in texts.c at the moment)
        choice_blocks = re.finditer(r'const\s+ChoiceItem\s+(\w+_choice\d+)\[\]\s*=\s*{(.*?)};', content, re.DOTALL)
        for block in choice_blocks:
            block_name = block.group(1)
            items = extract_items(block.group(2))
            texts = []
            for item in items:
                opts = parse_choice_item(item)
                texts.append(opts)
            choice_texts[block_name.upper()] = texts

    return dialog_texts, choice_texts, cluster_texts


def update_source_file(c_file, dialog_texts, choice_texts, cluster_texts):
    """
    Update the given C source file by adding comments with dialog/choice texts
    to lines calling talk_dialog() and choice_dialog().
    """
    modified = False  
    with open(c_file, 'r', encoding='utf-8') as f:  
        lines = f.readlines()  

    new_lines = []  
    # Regex to match talk_dialog, talk_cluster and choice_dialog calls
    talk_re = re.compile(r'(\s*)(.*?)(talk_dialog\s*\(\s*&dialogs\[(\w+)_DIALOG(?:\d+)?\]\[(\d+)\]\s*\);)(.*)?$')
    cluster_re = re.compile(r'(\s*)(.*?)(talk_cluster\s*\(\s*&dialog_clusters\[(\w+)\]\s*\);)(.*)?$')
    choice_re = re.compile(r'(\s*)(.*?)(choice_dialog\s*\(\s*&choices\[(\w+)_CHOICE(\d+)\]\[(\d+)\]\s*\);)(.*)?$')

    for line in lines:
        l = line.rstrip('\n')

        m = talk_re.search(l)
        if m:
            # Extract relevant groups from the match
            indent, before, call, act, idx, existing_comment = m.group(1), m.group(2), m.group(3), m.group(4), int(m.group(5)), m.group(6) or ""
            # Determine the dialog key
            key = "SYSTEM_DIALOG" if act.upper() == "SYSTEM" else None
            if not key:
                dnum_match = re.search(r'_DIALOG(\d+)', call)
                if dnum_match:
                    key = f"{act}_DIALOG{dnum_match.group(1)}".upper()
            texts = dialog_texts.get(key, [])
            if idx < len(texts):
                # Add comment with ES and EN text
                comment = f' // (ES) "{texts[idx]["es"]}" - (EN) "{texts[idx]["en"]}"'
                l = f"{indent}{before}{call}{comment}"
                modified = True
        else:
            m = cluster_re.search(l)
            if m:
                indent, before, call, cluster_name, existing_comment = m.group(1), m.group(2), m.group(3), m.group(4), m.group(5) or ""
                texts = cluster_texts.get(cluster_name.upper(), [])
                if texts:
                    texts_joined = ', '.join([f'(ES) "{t["es"]}" - (EN) "{t["en"]}"' for t in texts])
                    comment = f' // {texts_joined}'
                    l = f"{indent}{before}{call}{comment}"
                    modified = True
            else:
                m = choice_re.search(l)
                if m:
                    # Extract relevant groups from the match
                    indent, before, call, act, choice_num, idx, existing_comment = m.group(1), m.group(2), m.group(3), m.group(4), m.group(5), int(m.group(6)), m.group(7) or ""
                    key = f"{act}_CHOICE{choice_num}".upper()
                    opts = choice_texts.get(key, [])
                    if idx < len(opts):
                        options = opts[idx]
                        options_text = ', '.join([f'(ES) "{o["es"]}" - (EN) "{o["en"]}"' for o in options])
                        comment = f' // {options_text}'
                        l = f"{indent}{before}{call}{comment}"
                        modified = True

        new_lines.append(l + "\n")  

    # Write the updated lines back to the file
    with open(c_file, 'w', encoding='utf-8', newline='') as f:  
        f.writelines(new_lines)  

    if modified:  
        print(f"Successfully updated comments in {c_file}")  
    else:  
        print(f"No comments were modified in {c_file}")  


def process_file(c_file):
    """
    Process a single C file: read text definitions and update the file.
    """
    print(f"Processing file: {c_file}")
    dialog_texts, choice_texts, cluster_texts = parse_texts(["src/texts.c", "src/texts_generated.c"])
    update_source_file(c_file, dialog_texts, choice_texts, cluster_texts)


def process_all_files():
    """
    Process all C source files in the src directory (except texts.c).
    """
    for root, _, files in os.walk("src"):
        for file in files:
            if file.endswith(".c") and file != "texts.c":
                c_file = os.path.join(root, file)
                process_file(c_file)


def main():  
    """
    Entry point: parse arguments and process files accordingly.
    """
    if len(sys.argv) < 2:  
        show_help()  
        return  
    if sys.argv[1] == "*":  
        process_all_files()  
    else:  
        c_file = sys.argv[1]  
        if not c_file.startswith("src/"):  
            c_file = os.path.join("src", c_file)  
        if not os.path.exists(c_file):  
            print(f"Error: File {c_file} not found")  
            return  
        process_file(c_file)  


if __name__ == "__main__":  
    main()

