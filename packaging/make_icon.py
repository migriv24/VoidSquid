"""make_icon.py - draws Void Squid's icon (squid.png, squid.ico). Run once; the outputs are checked in.

A squid in the deep: a navy disc with a few stars, a violet-to-cyan mantle with two
fins, two eyes, and curling tentacles. Drawn at 1024 px and scaled down, so it is
our own art and needs no licence note.
"""
import math, random
from PIL import Image, ImageDraw, ImageFilter

S = 1024
img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
d = ImageDraw.Draw(img)

# the deep: a rounded square with a radial glow
bg = Image.new("RGBA", (S, S), (0, 0, 0, 0))
bd = ImageDraw.Draw(bg)
bd.rounded_rectangle([24, 24, S - 24, S - 24], radius=210, fill=(9, 13, 34, 255))
glow = Image.new("RGBA", (S, S), (0, 0, 0, 0))
gd = ImageDraw.Draw(glow)
for i in range(40, 0, -1):
    r = i * 11
    a = int(5 + (40 - i) * 0.5)
    gd.ellipse([S / 2 - r, S * 0.42 - r, S / 2 + r, S * 0.42 + r], fill=(70, 60, 170, a))
mask = Image.new("L", (S, S), 0)
ImageDraw.Draw(mask).rounded_rectangle([24, 24, S - 24, S - 24], radius=210, fill=255)
bg = Image.alpha_composite(bg, Image.composite(glow, Image.new("RGBA", (S, S), (0, 0, 0, 0)), mask))
img = Image.alpha_composite(img, bg)
d = ImageDraw.Draw(img)

random.seed(7)
for _ in range(70):
    x, y = random.uniform(90, S - 90), random.uniform(90, S - 90)
    r = random.choice([2, 2, 3, 4, 6])
    a = random.randint(120, 255)
    d.ellipse([x - r, y - r, x + r, y + r], fill=(220, 230, 255, a))

def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(len(a)))

violet, cyan = (158, 122, 240, 255), (86, 214, 222, 255)

# tentacles: thick curling strokes, drawn first so the mantle sits over their roots
tent = Image.new("RGBA", (S, S), (0, 0, 0, 0))
td = ImageDraw.Draw(tent)
cx, base = S / 2, S * 0.60
for k in range(8):
    off = (k - 3.5) / 3.5
    pts = []
    for j in range(60):
        t = j / 59
        x = cx + off * 150 * (1 - 0.3 * t) + math.sin(t * 3.2 + k) * 60 * t * (1 if k % 2 else -1)
        y = base + t * 300
        pts.append((x, y))
    for j in range(len(pts) - 1):
        t = j / (len(pts) - 1)
        w = int(34 * (1 - t) + 6)
        col = lerp(violet, cyan, 0.4 + 0.6 * t)
        td.line([pts[j], pts[j + 1]], fill=col, width=w)
        td.ellipse([pts[j][0] - w / 2, pts[j][1] - w / 2, pts[j][0] + w / 2, pts[j][1] + w / 2], fill=col)
img = Image.alpha_composite(img, Image.composite(tent, Image.new("RGBA", (S, S), (0, 0, 0, 0)), mask))

# the mantle: a tall rounded body with fins, shaded top to bottom
body = Image.new("RGBA", (S, S), (0, 0, 0, 0))
bdy = ImageDraw.Draw(body)
top, bottom, half = S * 0.14, S * 0.64, 190
for i in range(int(top), int(bottom)):
    t = max(0.0, (i - top) / (bottom - top))
    w = half * math.sin(min(1.0, t * 1.15) * math.pi / 2) ** 0.8
    if t < 0.12:
        w = half * (t / 0.12) ** 0.6 * 0.55
    col = lerp(violet, cyan, t)
    bdy.line([(cx - w, i), (cx + w, i)], fill=col)
# fins
bdy.polygon([(cx - 150, S * 0.20), (cx - 290, S * 0.13), (cx - 175, S * 0.36)], fill=violet)
bdy.polygon([(cx + 150, S * 0.20), (cx + 290, S * 0.13), (cx + 175, S * 0.36)], fill=violet)
img = Image.alpha_composite(img, body)
d = ImageDraw.Draw(img)

# eyes
for ex in (cx - 80, cx + 80):
    ey = S * 0.50
    d.ellipse([ex - 50, ey - 56, ex + 50, ey + 56], fill=(245, 248, 255, 255))
    d.ellipse([ex - 26, ey - 22, ex + 26, ey + 34], fill=(12, 16, 40, 255))
    d.ellipse([ex - 12, ey - 14, ex + 2, ey], fill=(255, 255, 255, 255))
# a highlight on the mantle
hl = Image.new("RGBA", (S, S), (0, 0, 0, 0))
ImageDraw.Draw(hl).ellipse([cx - 120, S * 0.20, cx - 40, S * 0.36], fill=(255, 255, 255, 60))
img = Image.alpha_composite(img, hl.filter(ImageFilter.GaussianBlur(14)))

img.resize((256, 256), Image.LANCZOS).save("squid.png")
img.save("squid.ico", sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
print("ok")
