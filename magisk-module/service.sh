MODDIR="${0%/*}"
MODNAME="${MODDIR##*/}"
MAGISKTMP="$(magisk --path)" || MAGISKTMP=/sbin

PROPFILE="$MAGISKTMP/.magisk/modules/$MODNAME/module.prop"
TMPFILE="$MAGISKTMP/dynmount.prop"
cp -af "$MODDIR/module.prop" "$TMPFILE"

sed -Ei 's/^description=(\[.*][[:space:]]*)?/description=[ ⛔ Proc monitor is not running. ] /g' "$TMPFILE"
flock "$MODDIR/module.prop"

mount --bind "$TMPFILE" "$PROPFILE"

chmod 777 "$MODDIR/proc_monitor"
unshare -m "$MODDIR/proc_monitor" --start "$MAGISKTMP"
exit 0