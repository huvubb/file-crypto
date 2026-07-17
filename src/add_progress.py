path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\main.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Add progress function
progress_fn = '''
static void ShowProgress(const std::string& label, size_t done, size_t total) {
    if (total == 0) return;
    int pct = static_cast<int>(done * 100 / total);
    static int lastPct = -1;
    if (pct != lastPct && pct % 10 == 0) {
        lastPct = pct;
        std::cout << "\\r" << label << " " << pct << "%" << std::flush;
    }
    if (done >= total) { std::cout << "\\r" << label << " 100%\\n"; lastPct = -1; }
}

'''

# Insert after TrimPath
marker = 'static std::string TrimPath'
content = content.replace(marker, progress_fn + marker)

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
print("Added progress function")
