path = r"C:\Users\邓涛\Documents\New project 3\file-crypto\src\main.cpp"
with open(path, "r", encoding="utf-8") as f:
    content = f.read()

# Replace global gRng with lazy-init inside RandomSuffix
old = """static std::mt19937 gRng(static_cast<unsigned>(
    std::chrono::steady_clock::now().time_since_epoch().count()));

static std::string RandomSuffix() {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string s(6, ' ');
    std::uniform_int_distribution<int> dist(0, 35);
    for (int i = 0; i < 6; ++i) s[i] = chars[dist(gRng)];
    return s;
}"""

new = """static std::string RandomSuffix() {
    static std::mt19937 rng(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string s(6, ' ');
    std::uniform_int_distribution<int> dist(0, 35);
    for (int i = 0; i < 6; ++i) s[i] = chars[dist(rng)];
    return s;
}"""

if old in content:
    content = content.replace(old, new)
    print("Patched RNG to lazy-init")
else:
    print("WARNING: old pattern not found!")

with open(path, "w", encoding="utf-8") as f:
    f.write(content)
