# LaTeX Report - Group 90

## Cấu trúc thư mục

```
latex/
├── main.tex              # File chính
├── preamble.tex          # Packages và settings
├── sections/
│   ├── titlepage.tex     # Trang bìa
│   ├── introduction.tex  # Giới thiệu
│   ├── theory.tex        # Cơ sở lý thuyết
│   ├── implementation.tex # Thiết kế và triển khai
│   ├── results.tex       # Kết quả thực nghiệm
│   ├── conclusion.tex    # Kết luận
│   └── references.tex    # Tài liệu tham khảo
├── figures/              # Hình ảnh minh họa
└── README.md             # Hướng dẫn này
```

## Hướng dẫn Compile

### Sử dụng pdflatex (command line)
```bash
cd latex
pdflatex main.tex
pdflatex main.tex   # Chạy 2 lần để update mục lục
```

### Sử dụng Overleaf
1. Upload toàn bộ folder `latex/` lên Overleaf
2. Đảm bảo folder `output/` cũng được upload (chứa charts)
3. Set `main.tex` là main document
4. Compile

### Sử dụng VS Code + LaTeX Workshop
1. Cài extension "LaTeX Workshop"
2. Mở folder `latex/`
3. Nhấn Ctrl+Alt+B để build

## Lưu ý

- Hình ảnh từ folder `output/` sẽ được tự động tìm (xem `preamble.tex`)
- Điền thông tin nhóm trong `sections/titlepage.tex`
- Mỗi section là một file riêng để dễ quản lý

