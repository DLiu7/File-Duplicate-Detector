# 🧩 File Duplicate Detector

A C-based tool for recursively scanning directories and detecting **true duplicate files** based on **MD5 hashing**, grouping them by hard and symbolic links.

This project was developed as part of the CS214 Systems Programming course at Rutgers.

---

## 🚀 Features

- ✅ Detects true duplicates using OpenSSL-based MD5 hashing  
- 📁 Recursively traverses directory trees using `nftw()`  
- 📎 Groups identical files by:
  - Inodes (hard links)
  - Symlinks (soft links to existing files)
- 🧹 Clean and structured output with paths under each group

---

## 🛠️ Build Instructions

You need:
- A C compiler (e.g. `gcc`)
- OpenSSL development libraries (e.g. `libssl-dev`)

Then run:

```bash
make
```

To execute:
```bash
./detect_dups <directory>
```
