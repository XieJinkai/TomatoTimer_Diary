import sys
import subprocess
import os

os.environ.setdefault("PADDLE_PDX_DISABLE_MODEL_SOURCE_CHECK", "True")
os.environ.setdefault("FLAGS_use_mkldnn", "0")
os.environ.setdefault("FLAGS_enable_pir_api", "0")

def extract_texts(result):
    texts = []
    if result is None:
        return texts
    if isinstance(result, dict):
        for key in ("rec_texts", "texts", "text"):
            value = result.get(key)
            if isinstance(value, list):
                for item in value:
                    if isinstance(item, str) and item.strip():
                        texts.append(item.strip())
            elif isinstance(value, str) and value.strip():
                texts.append(value.strip())
        for value in result.values():
            texts.extend(extract_texts(value))
        return texts
    if isinstance(result, list):
        for block in result:
            if block is None:
                continue
            if isinstance(block, list):
                for line in block:
                    if isinstance(line, (list, tuple)) and len(line) >= 2:
                        right = line[1]
                        if isinstance(right, (list, tuple)) and len(right) >= 1:
                            t = right[0]
                            if isinstance(t, str) and t.strip():
                                texts.append(t.strip())
                    elif isinstance(line, dict):
                        texts.extend(extract_texts(line))
            elif isinstance(block, (list, tuple)) and len(block) >= 2:
                right = block[1]
                if isinstance(right, (list, tuple)) and len(right) >= 1:
                    t = right[0]
                    if isinstance(t, str) and t.strip():
                        texts.append(t.strip())
            elif isinstance(block, dict):
                texts.extend(extract_texts(block))
    return texts

def main():
    if len(sys.argv) < 2:
        print("")
        return
    img = sys.argv[1]
    try:
        from paddleocr import PaddleOCR
    except Exception as e:
        try:
            subprocess.run(
                [sys.executable, "-m", "pip", "install", "-U", "paddleocr", "--user"],
                check=True
            )
            from paddleocr import PaddleOCR
        except Exception as install_err:
            print(f"无法导入 paddleocr: {e}; 自动安装失败: {install_err}")
            return
    try:
        candidates = [
            {"lang": "ch", "show_log": False, "use_gpu": False, "enable_mkldnn": False, "use_angle_cls": True},
            {"lang": "ch", "use_gpu": False, "enable_mkldnn": False, "use_angle_cls": True},
            {"lang": "ch", "show_log": False, "enable_mkldnn": False, "use_textline_orientation": True},
            {"lang": "ch", "enable_mkldnn": False, "use_textline_orientation": True},
            {"lang": "ch", "show_log": False, "enable_mkldnn": False},
            {"lang": "ch", "enable_mkldnn": False},
            {"lang": "ch", "show_log": False, "use_gpu": False, "use_angle_cls": True},
            {"lang": "ch", "use_gpu": False, "use_angle_cls": True},
            {"lang": "ch", "show_log": False, "use_textline_orientation": True},
            {"lang": "ch", "use_textline_orientation": True},
            {"lang": "ch"},
        ]
        ocr = None
        last_err = None
        for kw in candidates:
            try:
                ocr = PaddleOCR(**kw)
                break
            except Exception as e:
                last_err = e
        if ocr is None:
            raise last_err if last_err else RuntimeError("PaddleOCR 初始化失败")
        result = None
        errs = []
        for fn in (
            lambda: ocr.predict(img),
            lambda: ocr.ocr(img),
            lambda: ocr.ocr(img, cls=True),
        ):
            try:
                result = fn()
                break
            except Exception as e:
                errs.append(str(e))
        if result is None:
            raise RuntimeError(" | ".join(errs[-3:]))
        texts = extract_texts(result)
        print("\n".join(texts))
    except Exception as e:
        print(f"OCR 执行失败: {e}")

if __name__ == "__main__":
    main()
