lv_i18n usage:
windows命令行下，切换目录到gui
1. lv_i18n extract -s "view/**/*.+(c|cpp|h|hpp)" -t "view/i18n/config/*.yml"
2. lv_i18n compile -t "view/i18n/config/*.yml" -o "view/i18n/translations/"