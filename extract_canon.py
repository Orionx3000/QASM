import os
from docx import Document
import PyPDF2

def extract_docx(file_path):
    doc = Document(file_path)
    return "\n".join([p.text for p in doc.paragraphs])

def extract_pdf(file_path):
    text = ""
    with open(file_path, 'rb') as f:
        reader = PyPDF2.PdfReader(f)
        for page in reader.pages:
            t = page.extract_text()
            if t: text += t + "\n"
    return text

files = [
    r"D:\500my Land of lost trees\500million years-\500million years\_DC published work\Textbooks\Educational isomophism in the bala tor system.pdf",
    r"D:\500my Land of lost trees\500million years-\500million years\_DC published work\Textbooks\Bala tor kids charts.docx",
    r"D:\500my Land of lost trees\500million years-\500million years\_DC published work\Textbooks\The Secret Teachings of Apara Den Vivarana_ An Esoteric Compendium.docx",
    r"D:\500my Land of lost trees\500million years-\500million years\_DC published work\Textbooks\_Bala Tor di - the Master Count.pdf"
]

out_dir = r"D:\App Creation\QASM\canon_texts"
os.makedirs(out_dir, exist_ok=True)

for path in files:
    name = os.path.basename(path)
    print(f"Extracting {name}...")
    try:
        if name.endswith('.docx'):
            text = extract_docx(path)
        elif name.endswith('.pdf'):
            text = extract_pdf(path)
        else:
            continue
            
        out_path = os.path.join(out_dir, name + ".txt")
        with open(out_path, 'w', encoding='utf-8') as out_f:
            out_f.write(text)
        print(f"  -> Saved to {out_path} ({len(text)} chars)")
    except Exception as e:
        print(f"  -> ERROR: {e}")
