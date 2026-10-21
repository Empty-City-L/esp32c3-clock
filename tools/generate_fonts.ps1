# 重新生成 LVGL 字体（可选）
#
# 只有修改界面文案、需要新增汉字时才需要运行。
# 生成结果已经放在 src/fonts/，直接编译即可。
#
# 依赖：
#   Node.js 18+
#   npm i -g lv_font_conv
#
# 用法（在项目根目录）：
#   powershell -ExecutionPolicy Bypass -File tools/generate_fonts.ps1
# 或指定 lv_font_conv 的完整路径：
#   powershell -ExecutionPolicy Bypass -File tools/generate_fonts.ps1 -LvFontConv "C:\...\lv_font_conv.CMD"

param(
    [string]$LvFontConv = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

if (-not $LvFontConv) {
    $cmd = Get-Command lv_font_conv -ErrorAction SilentlyContinue
    if (-not $cmd) {
        throw "找不到 lv_font_conv。请先运行：npm i -g lv_font_conv"
    }
    $LvFontConv = $cmd.Source
}

$din = "C:\Windows\Fonts\DINNextLTPro-Bold.ttf"
$cn  = "C:\Windows\Fonts\Deng.ttf"
$sym = "C:\Windows\Fonts\seguisym.ttf"

if (-not (Test-Path $din)) { throw "找不到 DIN Next LT Pro Bold：$din" }
if (-not (Test-Path $cn))  { throw "找不到等线 Deng.ttf：$cn" }
if (-not (Test-Path $sym)) { throw "找不到 Segoe UI Symbol：$sym" }

# 界面里用到的全部汉字 + 标点。新增文案时把新字补进来。
# 天气相关：城市名、天气现象、风向、状态文案。
$symbols = "周六日一二三四五六七八九十零月年天时分秒正在连接同步对失败重试中配网模式浏览器访问热点打开地址已离线走时获取无线网络设置成功保存重启恢复出厂确认取消提示错误输入选择扫描附近等待当前星期上午下午点气温湿度城市更新刷新加载完毕欢迎使用屏幕亮度方向偏移颜色反转校准设备信息主控版本固件检查信号服务后稍候·…刚刚北京上海广州深圳杭州南京成都重庆武汉西安苏州天津长沙郑州青岛大连宁波厦门福州昆明合肥济南沈阳哈尔滨长春石家庄太原南昌贵阳南宁海口兰州银川西宁乌鲁木齐呼和浩特拉萨邵阳衡阳岳阳常德株洲湘潭郴州永州怀化娄底益阳张家界多云阴晴阵雨雷小中大暴雪雾霾沙尘未知东南西北风级天气已更新分钟前未配置请在网页填写心知暂立即到冰雹夹浮扬强浓特重严中度今明后密码或参数效权限额尽求过于频繁再结果关闭，、°力间冻"

function New-Font {
    param(
        [string]$FontFile,
        [int]$Size,
        [string]$OutFile,
        [string]$Symbols,
        [string]$Range
    )
    $argv = @(
        "--font", $FontFile,
        "--size", "$Size",
        "--bpp", "4",
        "--format", "lvgl",
        "--no-compress",
        "--no-kerning",
        "--lv-include", "lvgl.h",
        "-o", $OutFile
    )
    if ($Range)   { $argv += @("--range", $Range) }
    if ($Symbols) { $argv += @("--symbols", $Symbols) }
    & $LvFontConv @argv
    if ($LASTEXITCODE -ne 0) { throw "生成字体失败：$OutFile" }
}

New-Font $din 44 "src/fonts/font_time_44.c" "°" "0x30-0x3A"
New-Font $din 30 "src/fonts/font_din_30.c"  "°" "0x30-0x3A"
New-Font $din 22 "src/fonts/font_din_22.c"  "°" "0x30-0x3A"
New-Font $din 16 "src/fonts/font_sec_16.c"  "" "0x30-0x3A"
New-Font $cn  14 "src/fonts/font_cn_14.c"   $symbols "0x20-0x7E"
New-Font $cn  12 "src/fonts/font_cn_12.c"   $symbols "0x20-0x7E"
New-Font $sym 28 "src/fonts/font_weather_28.c" "☀⛅☁☔❄⚡≋?" ""
New-Font $sym 18 "src/fonts/font_weather_18.c" "☀⛅☁☔❄⚡≋?" ""

Write-Host "字体已更新：src/fonts/*.c" -ForegroundColor Green
