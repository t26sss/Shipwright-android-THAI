#!/bin/sh
# ใช้: sh apply.sh /path/to/โปรเจกต์   (หรือ cd เข้าโฟลเดอร์โปรเจกต์แล้วรัน sh /path/to/apply.sh)
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="${1:-$(pwd)}"
cd "$REPO"
if [ ! -d Android ] || [ ! -d soh ]; then
  echo "ไม่ใช่โฟลเดอร์รากของโปรเจกต์ (ต้องมีโฟลเดอร์ Android และ soh): $REPO"
  exit 1
fi

# 1) แก้ z_player.c และ GIVanillaBehavior.h ด้วย patch (ทำตามลำดับ ข้ามอันที่ใส่แล้ว)
for P in roll-game-logic.patch roll-game-logic-2.patch; do
  if git apply --check "$HERE/$P" 2>/dev/null; then
    git apply "$HERE/$P"
    echo "[ok] patch $P"
  elif git apply --check -R "$HERE/$P" 2>/dev/null; then
    echo "[skip] $P ใส่ไว้แล้ว"
  else
    echo "[ผิดพลาด] $P ใส่ไม่ได้ (z_player.c ถูกแก้ไปจากเดิม?) หยุดก่อนที่จะแตะไฟล์อื่น"
    exit 1
  fi
done

# 2) คัดลอกไฟล์ (สำรองของเดิมเป็น .bak ก่อน)
cd "$HERE/files"
find . -type f | while read -r f; do
  dest="$REPO/${f#./}"
  mkdir -p "$(dirname "$dest")"
  if [ -f "$dest" ] && [ ! -f "$dest.bak" ]; then cp "$dest" "$dest.bak"; fi
  cp "$f" "$dest"
  echo "[ok] ${f#./}"
done
echo
echo "เสร็จแล้ว ตรวจด้วย: git status"
