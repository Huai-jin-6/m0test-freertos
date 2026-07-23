#!/bin/bash
# ============================================================
#  MSPM0 FreeRTOS 工程模板 — 团队版
#
#  两种用法:
#
#   1. 在模板目录里直接运行 (你和已有仓库的队友):
#        ./new_project.sh <工程名> <目标目录>
#
#   2. 没有模板时, 把本脚本单独复制到任意位置运行:
#        ./new_project.sh --bootstrap
#      会自动引导克隆模板仓库，然后创建工程。
#
#  选项:
#    --bootstrap           强制进入引导模式 (克隆模板)
#    --repo <url>          Git 仓库地址
#    --template <path>     模板存放路径
#    --toolchain <path>    工具链目录
#    --sdk <path>          SDK 目录
#    --sysconfig <path>    SysConfig 目录
#    --reset               清除所有保存的配置
# ============================================================

set -e

# 确保基本命令可用 (Git Bash 可能缺少 PATH)
export PATH="/usr/bin:/bin:$PATH"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LAUNCH_DIR="$SCRIPT_DIR"   # 记住用户运行脚本的原始位置
USER_CONF="$HOME/.mspm0_template.conf"

# ==================== 配置文件 (优先本目录, 其次用户目录) ====================

load_conf() {
    TOOLCHAIN_GCC=""
    TOOLCHAIN_GXX=""
    SDK_PATH=""
    SYSCONFIG_DIR=""
    TEMPLATE_DIR=""
    REPO_URL=""

    # 先加载用户级配置
    if [ -f "$USER_CONF" ]; then
        source "$USER_CONF"
    fi
    # 再加载工程级配置 (覆盖)
    if [ -f "$SCRIPT_DIR/.new_project.conf" ]; then
        source "$SCRIPT_DIR/.new_project.conf"
    fi

    # Auto-clear outdated repo URL cache
    if [ "$REPO_URL" = "https://github.com/Rh-i/MSPM0_FreeRTOS_PIDTEST" ]; then
        echo "  ⚠ Detected old template repo URL, auto-resetting"
        REPO_URL=""
        save_user_conf
    fi
    # Auto-convert SSH URL to HTTPS (SSH not configured for most teammates)
    if echo "$REPO_URL" | grep -q '^git@github\.com'; then
        REPO_URL=$(echo "$REPO_URL" | sed 's|^git@github\.com:|https://github.com/|')
        echo "  ⚠ Converted SSH to HTTPS: $REPO_URL"
        save_user_conf
    fi
}

save_user_conf() {
    cat > "$USER_CONF" << EOF
# MSPM0 工程模板 — 本机环境配置 (自动生成)
TOOLCHAIN_GCC="$TOOLCHAIN_GCC"
TOOLCHAIN_GXX="$TOOLCHAIN_GXX"
SDK_PATH="$SDK_PATH"
SYSCONFIG_DIR="$SYSCONFIG_DIR"
TEMPLATE_DIR="$TEMPLATE_DIR"
REPO_URL="$REPO_URL"
EOF
}

load_conf

# ==================== 检查是否在模板目录内 ====================

is_template_dir() {
    local dir="$1"
    [ -f "$dir/CMakeLists.txt" ] && [ -f "$dir/mspm0g350x_base.cmake" ] && [ -d "$dir/FreeRTOS" ] && [ -d "$dir/User" ] && [ -d "$dir/SysConfig" ]
}

# ==================== 解析命令行 ====================

NEW_NAME=""
NEW_DIR=""
BOOTSTRAP=false

# 如果不在模板目录内, 自动进入 bootstrap 模式
if ! is_template_dir "$SCRIPT_DIR"; then
    BOOTSTRAP=true
fi

while [ $# -gt 0 ]; do
    case "$1" in
        --bootstrap)   BOOTSTRAP=true;  shift ;;
        --repo)        REPO_URL="$2";   shift 2 ;;
        --template)    TEMPLATE_DIR="$2"; shift 2 ;;
        --toolchain)   TOOLCHAIN_GCC="$2/bin/arm-none-eabi-gcc.exe"
                       TOOLCHAIN_GXX="$2/bin/arm-none-eabi-g++.exe"
                       shift 2 ;;
        --sdk)         SDK_PATH="$2";    shift 2 ;;
        --sysconfig)   SYSCONFIG_DIR="$2"; shift 2 ;;
        --reset)       rm -f "$USER_CONF" "$SCRIPT_DIR/.new_project.conf"
                       echo "配置已重置"; exit 0 ;;
        --help)
            head -30 "$0"
            exit 0 ;;
        -*)
            echo "[错误] 未知选项: $1"; exit 1 ;;
        *)
            if [ -z "$NEW_NAME" ]; then NEW_NAME="$1"
            elif [ -z "$NEW_DIR" ]; then NEW_DIR="$1"
            else echo "[警告] 忽略多余参数: $1"; fi
            shift ;;
    esac
done

# ==================== Bootstrap 模式 ====================

if $BOOTSTRAP; then

    echo "============================================"
    echo "  检测到当前不在模板目录中"
    echo "  进入引导模式 —— 下载模板仓库"
    echo "============================================"
    echo

    # --- Git 仓库地址 ---
    if [ -z "$REPO_URL" ]; then
        printf "Git 仓库地址 [https://github.com/Huai-jin-6/m0template.git]: " >&2
        read -r input
        REPO_URL="${input:-${REPO_URL:-https://github.com/Huai-jin-6/m0template.git}}"
    fi
    echo "  仓库: $REPO_URL"

    # --- 模板存放位置 ---
    if [ -z "$TEMPLATE_DIR" ]; then
        printf "模板存放目录 [${HOME}/MSPM0_Template]: " >&2
        read -r input
        TEMPLATE_DIR="${input:-${TEMPLATE_DIR:-$HOME/MSPM0_Template}}"
    fi
    echo "  模板: $TEMPLATE_DIR"
    echo

    # --- 克隆 ---
    if [ -d "$TEMPLATE_DIR" ]; then
        echo "  ✓ 模板目录已存在，跳过克隆"
    else
        echo "  正在克隆..."
        git clone "$REPO_URL" "$TEMPLATE_DIR"
        echo "  ✓ 克隆完成"
    fi

    # --- 保存并切换到模板目录 ---
    SCRIPT_DIR="$TEMPLATE_DIR"
    save_user_conf

    # 把本脚本复制到模板里 (方便以后直接用)
    if [ ! -f "$TEMPLATE_DIR/new_project.sh" ]; then
        cp "$0" "$TEMPLATE_DIR/new_project.sh"
        echo "  ✓ 脚本已复制到 $TEMPLATE_DIR/new_project.sh"
    fi

    echo "  以后可以直接: cd $TEMPLATE_DIR && ./new_project.sh 工程名 目录"
    echo

    # --- 如果用户已经指定了工程名, 继续创建; 否则交互询问 ---
    if [ -z "$NEW_NAME" ]; then
        echo
        echo "模板已就绪。现在创建新工程:"
        echo
        printf "  工程名 (如 MyMotor): " >&2
        read -r NEW_NAME
        printf "  存放目录 [%s]: " "$LAUNCH_DIR/${NEW_NAME}" >&2
        read -r NEW_DIR
        NEW_DIR="${NEW_DIR:-$LAUNCH_DIR/${NEW_NAME}}"
    fi
fi

# ==================== 此时 SCRIPT_DIR 必须是有效模板目录 ====================

TEMPLATE_DIR="$SCRIPT_DIR"

if ! is_template_dir "$TEMPLATE_DIR"; then
    echo "[错误] 模板目录无效: $TEMPLATE_DIR"
    echo "  请用 --bootstrap 先下载模板"
    exit 1
fi

CONF_FILE="$TEMPLATE_DIR/.new_project.conf"

# ==================== 参数检查 ====================

if [ -z "$NEW_NAME" ] || [ -z "$NEW_DIR" ]; then
    echo
    echo "创建新工程:"
    echo
    if [ -z "$NEW_NAME" ]; then
        printf "  工程名 (如 MyMotor): " >&2
        read -r NEW_NAME
    fi
    if [ -z "$NEW_DIR" ]; then
        DEFAULT_DIR="$LAUNCH_DIR/${NEW_NAME}"
        printf "  存放目录 [%s]: " "$DEFAULT_DIR" >&2
        read -r input
        NEW_DIR="${input:-$DEFAULT_DIR}"
    fi
fi

if echo "$NEW_NAME" | grep -qvE '^[A-Za-z_][A-Za-z0-9_]*$'; then
    echo "[错误] 工程名只能包含字母、数字、下划线，且不能以数字开头"
    exit 1
fi

if [ -d "$NEW_DIR" ]; then
    echo "[错误] 目录 '$NEW_DIR' 已存在"
    exit 1
fi

# ==================== 交互式引导 ====================

prompt_path() {
    local prompt="$1" current="$2" check="$3" input=""
    while true; do
        if [ -n "$current" ]; then
            printf "%s [%s]: " "$prompt" "$current" >&2
        else
            printf "%s: " "$prompt" >&2
        fi
        read -r input
        [ -z "$input" ] && input="$current"
        [ -z "$input" ] && continue
        if [ -n "$check" ] && [ ! -e "$input" ]; then
            echo "  ✗ 找不到: $input" >&2
            printf "  强制使用? (y/n): " >&2
            read -r force
            [ "$force" = "y" ] || [ "$force" = "Y" ] && { echo "$input"; return; }
            current=""
            continue
        fi
        echo "$input"
        return
    done
}

skip_prompt=false
if [ -n "$TOOLCHAIN_GCC" ] && [ -n "$SDK_PATH" ] && [ -n "$SYSCONFIG_DIR" ]; then
    if [ -f "$TOOLCHAIN_GCC" ] && [ -d "$SDK_PATH" ] && [ -f "$SYSCONFIG_DIR/sysconfig_gui.bat" ]; then
        skip_prompt=true
    fi
fi

if [ "$skip_prompt" = false ]; then
    echo "============================================"
    echo "  🔧 首次配置 — 引导设置 (仅此一次)"
    echo "============================================"

    [ -z "$TOOLCHAIN_GCC" ] && TOOLCHAIN_GCC="D:/Toolchain/arm-gnu-toolchain-15.2.rel1/bin/arm-none-eabi-gcc.exe"
    [ -z "$TOOLCHAIN_GXX" ] && TOOLCHAIN_GXX="D:/Toolchain/arm-gnu-toolchain-15.2.rel1/bin/arm-none-eabi-g++.exe"
    TOOLCHAIN_DIR=$(dirname "$(dirname "$TOOLCHAIN_GCC")")
    TOOLCHAIN_DIR=$(prompt_path "ARM GCC 工具链目录 (包含 bin/)" "$TOOLCHAIN_DIR")
    TOOLCHAIN_GCC="$TOOLCHAIN_DIR/bin/arm-none-eabi-gcc.exe"
    TOOLCHAIN_GXX="$TOOLCHAIN_DIR/bin/arm-none-eabi-g++.exe"

    [ -z "$SDK_PATH" ] && SDK_PATH="C:/ti/mspm0_sdk_2_10_00_04"
    SDK_PATH=$(prompt_path "MSPM0 SDK 目录" "$SDK_PATH" "$SDK_PATH")

    [ -z "$SYSCONFIG_DIR" ] && SYSCONFIG_DIR="D:/CCS/sysconfig"
    SYSCONFIG_DIR=$(prompt_path "SysConfig 安装目录" "$SYSCONFIG_DIR" "$SYSCONFIG_DIR/sysconfig_gui.bat")

    save_user_conf
    echo "  ✓ 已保存"
    echo
fi

# ==================== 环境信息 ====================

echo "============================================"
echo "  MSPM0 FreeRTOS — 新建工程"
echo "============================================"
echo "  模板:       $TEMPLATE_DIR"
echo "  工程名:     $NEW_NAME"
echo "  目标:       $NEW_DIR"
echo "  工具链:     $TOOLCHAIN_GCC"
echo "  SDK:        $SDK_PATH"
echo "  SysConfig:  $SYSCONFIG_DIR"
echo "============================================"
echo

# ==================== [1/4] 复制文件 ====================

echo "[1/4] 复制文件..."

mkdir -p "$NEW_DIR"
cp "$TEMPLATE_DIR/CMakeLists.txt"         "$NEW_DIR/"
cp "$TEMPLATE_DIR/mspm0g350x_base.cmake"  "$NEW_DIR/"
cp "$TEMPLATE_DIR/.gitignore"             "$NEW_DIR/"
# 生成 clangd 配置文件 (不依赖模板仓库)
cat > "$NEW_DIR/.clangd" << 'CLANGD_EOF'
CompileFlags:
  CompilationDatabase: build/
  Remove:
    - -mcpu=*
    - -march=*
    - -mthumb
    - -mfloat-abi=*
    - -gstrict-dwarf
    - -fmessage-length=*

Diagnostics:
  Suppress:
    - unused-parameter
    - missing-prototypes
  UnusedIncludes: Strict
  ClangTidy:
    Remove:
      - modernize-use-trailing-return-type
CLANGD_EOF
test -f "$TEMPLATE_DIR/.clang-format" && cp "$TEMPLATE_DIR/.clang-format" "$NEW_DIR/" || true

cp -r "$TEMPLATE_DIR/FreeRTOS/" "$NEW_DIR/FreeRTOS/"
rm -rf "$NEW_DIR/FreeRTOS/examples" \
       "$NEW_DIR/FreeRTOS/portable/IAR" \
       "$NEW_DIR/FreeRTOS/portable/RVDS" \
       "$NEW_DIR/FreeRTOS/portable/CCS" \
       "$NEW_DIR/FreeRTOS/portable/Softune" \
       "$NEW_DIR/FreeRTOS/portable/Renesas" \
       "$NEW_DIR/FreeRTOS/portable/ThirdParty"
rm -f  "$NEW_DIR/FreeRTOS/portable/MemMang/heap_"{1,2,3,5}".c"
rm -f  "$NEW_DIR/FreeRTOS/CMakeLists.txt"

cp -r "$TEMPLATE_DIR/User/"      "$NEW_DIR/User/"
cp -r "$TEMPLATE_DIR/Flash/"     "$NEW_DIR/Flash/"
cp -r "$TEMPLATE_DIR/SysConfig/" "$NEW_DIR/SysConfig/"
test -d "$TEMPLATE_DIR/.vscode" && cp -r "$TEMPLATE_DIR/.vscode/" "$NEW_DIR/.vscode/" || true
rm -f "$NEW_DIR/User/Service/"*.d 2>/dev/null || true

# ==================== [2/4] 生成 CMakePresets.json ====================

echo "[2/4] 生成 CMakePresets.json..."

# 转义反斜杠为 JSON 兼容格式
GCC_JSON=$(echo "$TOOLCHAIN_GCC" | sed 's|\\|/|g')
GXX_JSON=$(echo "$TOOLCHAIN_GXX" | sed 's|\\|/|g')

cat > "$NEW_DIR/CMakePresets.json" << JSONEOF
{
    "version": 8,
    "configurePresets": [
        {
            "name": "debug",
            "displayName": "GCC arm-none-eabi",
            "description": "C: ${GCC_JSON}, CXX: ${GXX_JSON}",
            "generator": "Ninja",
            "binaryDir": "\${sourceDir}/build",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "${GCC_JSON}",
                "CMAKE_CXX_COMPILER": "${GXX_JSON}",
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
            }
        }
    ]
}
JSONEOF

# 统一正斜杠，避免 sed/CMake 转义问题
SDK_CMAKE=$(echo "$SDK_PATH" | sed 's|\\|/|g')
sed -i "s|^set(MSPM0_SDK_PATH .*|set(MSPM0_SDK_PATH  ${SDK_CMAKE})|" "$NEW_DIR/mspm0g350x_base.cmake"

true  # SysConfig path injected into CMakePresets.json above

# ==================== [3/4] 重命名 + 替换 ====================

echo "[3/4] 重命名 + 替换工程名..."

OLD_SYSCFG="$NEW_DIR/SysConfig/m0test.syscfg"
NEW_SYSCFG="$NEW_DIR/SysConfig/${NEW_NAME}.syscfg"
[ -f "$OLD_SYSCFG" ] && mv "$OLD_SYSCFG" "$NEW_SYSCFG"

sed -i "s|^project(\S* |project(${NEW_NAME} |" "$NEW_DIR/CMakeLists.txt"

# ==================== [4/4] 预留目录 ====================

echo "[4/4] 创建预留目录..."

mkdir -p "$NEW_DIR/User/Algorithm" "$NEW_DIR/User/Protocol" "$NEW_DIR/User/Module" \
         "$NEW_DIR/User/Device" "$NEW_DIR/User/Main" "$NEW_DIR/build"

# ==================== 完成 ====================

echo
echo "============================================"
echo "  ✓ 完成!"
echo "============================================"
echo "  cd $NEW_DIR && cmake --preset debug -B build && cmake --build build"
echo "============================================"
