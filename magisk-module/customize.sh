api_level_arch_detect
mkdir "$MODPATH/zygisk"
[ ! -d "$MODPATH/libs/$ABI" ] && abort "! $ABI not supported"
cp -af "$MODPATH/libs/$ABI/libproc_monitor.so" "$MODPATH/zygisk/$ABI.so"
cp -af "$MODPATH/libs/$ABI32/libproc_monitor.so" "$MODPATH/zygisk/$ABI32.so"
rm -rf "$MODPATH/libs"
