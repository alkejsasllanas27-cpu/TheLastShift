from PIL import Image, ImageDraw
from pathlib import Path
root=Path(__file__).parent
files=sorted((root/'thumbnails').glob('*.png'))
sheet=Image.new('RGB',(1200,((len(files)+7)//8)*126),'#cccccc')
d=ImageDraw.Draw(sheet)
for i,p in enumerate(files):
    img=Image.open(p).convert('RGB'); img.thumbnail((144,102))
    x=(i%8)*150;y=(i//8)*126
    sheet.paste(img,(x,y+20));d.text((x+4,y+3),p.stem,fill='black')
sheet.save(root/'contact.png')
