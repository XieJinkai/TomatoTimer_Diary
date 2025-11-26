import sys
from paddleocr import PaddleOCR
def main():
    if len(sys.argv) < 2:
        print("")
        return
    img = sys.argv[1]
    ocr = PaddleOCR(use_angle_cls=True, lang='ch')
    result = ocr.ocr(img, cls=True)
    texts = []
    for res in result:
        for line in res:
            texts.append(line[1][0])
    print("\n".join(texts))
if __name__ == '__main__':
    main()
