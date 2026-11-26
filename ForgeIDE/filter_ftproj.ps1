param(
    [string]$srcProj,   # 源 ftproj
    [string]$dstProj    # 目标 ftproj
)

# --------------------------------------------------------
# 检查文件存在
# --------------------------------------------------------
if (-not (Test-Path $srcProj)) {
    Write-Host "[ERROR] Source proj not found: $srcProj"
    exit 1
}
if (-not (Test-Path $dstProj)) {
    Write-Host "[ERROR] Target proj not found: $dstProj"
    exit 1
}

# --------------------------------------------------------
# 辅助函数：提取嵌套块
# --------------------------------------------------------
function Get-Block {
    param([string]$text, [int]$startPos, [string]$tag)
    $openTag  = "<$tag>"
    $closeTag = "</$tag>"
    $oLen = $openTag.Length
    $cLen = $closeTag.Length
    $depth = 0
    $i = $startPos
    $len = $text.Length
    while ($i -lt $len) {
        if (($i + $oLen -le $len) -and ($text.Substring($i, $oLen) -eq $openTag)) {
            $depth++
            $i += $oLen
        }
        elseif (($i + $cLen -le $len) -and ($text.Substring($i, $cLen) -eq $closeTag)) {
            $depth--
            $i += $cLen
            if ($depth -eq 0) {
                return @{ Content = $text.Substring($startPos, $i - $startPos); EndPos = $i }
            }
        }
        else { $i++ }
    }
    return $null
}

# --------------------------------------------------------
# 辅助函数：取某 tag 的内部内容（首个出现，不含标签本身）
# --------------------------------------------------------
function Get-InnerFirst {
    param([string]$text, [string]$tag)
    $openTag  = "<$tag>"
    $closeTag = "</$tag>"
    $idx = $text.IndexOf($openTag)
    if ($idx -lt 0) { return $null }
    $b = Get-Block $text $idx $tag
    if ($null -eq $b) { return $null }
    $content = $b.Content
    return $content.Substring($openTag.Length, $content.Length - $openTag.Length - $closeTag.Length)
}

# --------------------------------------------------------
# 辅助函数：替换某 tag 的内部内容（首个出现）
# --------------------------------------------------------
function Set-InnerFirst {
    param([string]$block, [string]$tag, [string]$newInner)
    $openTag = "<$tag>"
    $idx = $block.IndexOf($openTag)
    if ($idx -lt 0) { return $block }
    $b = Get-Block $block $idx $tag
    if ($null -eq $b) { return $block }
    $newSeg = $openTag + $newInner + "</$tag>"
    return $block.Substring(0, $idx) + $newSeg + $block.Substring($b.EndPos)
}

# --------------------------------------------------------
# 辅助函数：取 Configuration 的 Name 属性
# --------------------------------------------------------
function Get-ConfigName {
    param([string]$text)
    $m = [regex]::Match($text, '(?<![A-Za-z])Name="([^"]*)"')
    if ($m.Success) { return $m.Groups[1].Value }
    return $null
}

# --------------------------------------------------------
# 辅助函数：取出某 tag 的所有顶层块
# --------------------------------------------------------
function Get-AllBlocks {
    param([string]$text, [string]$tag)
    $list = @()
    $openTag = "<$tag>"
    $oLen = $openTag.Length
    $i = 0
    $len = $text.Length
    while ($i -lt $len) {
        if (($i + $oLen -le $len) -and ($text.Substring($i, $oLen) -eq $openTag)) {
            $b = Get-Block $text $i $tag
            if ($null -ne $b) { $list += $b; $i = $b.EndPos }
            else { $i++ }
        }
        else { $i++ }
    }
    return $list
}

# --------------------------------------------------------
# 核心函数：把 SRC 每个 Configuration 的
#   - Option->Compiler->General 内部内容
#   - RegisterConfig 内部内容
# 按 Configuration 的 Name 匹配拷贝到 DST
# --------------------------------------------------------
function Copy-ConfigSettings {
    param([string]$srcRaw, [string]$dstRaw)

    # 1. 建立 SRC 映射: Name -> @{ General; RegisterConfig }
    $srcMap = @{}
    foreach ($cb in (Get-AllBlocks $srcRaw 'Configuration')) {
        $name = Get-ConfigName $cb.Content
        if ([string]::IsNullOrEmpty($name)) { continue }
        $srcMap[$name] = @{
            General        = Get-InnerFirst $cb.Content 'General'
            RegisterConfig = Get-InnerFirst $cb.Content 'RegisterConfig'
        }
    }
    Write-Host "[INFO] SRC configurations: $($srcMap.Keys -join ', ')"

    # 2. 遍历 DST 的 Configuration，逐个替换
    $sb  = New-Object System.Text.StringBuilder
    $i   = 0
    $len = $dstRaw.Length
    while ($i -lt $len) {
        if (($i + 15 -le $len) -and ($dstRaw.Substring($i, 15) -eq '<Configuration>')) {
            $b = Get-Block $dstRaw $i 'Configuration'
            if ($null -eq $b) { [void]$sb.Append($dstRaw[$i]); $i++; continue }

            $blockText = $b.Content
            $name = Get-ConfigName $blockText

            if ($name -and $srcMap.ContainsKey($name)) {
                $src = $srcMap[$name]

                # 2a. 拷贝 Compiler->General 内部内容
                if ($null -ne $src.General) {
                    $blockText = Set-InnerFirst $blockText 'General' $src.General
                    Write-Host "[INFO] [$name] copied Compiler->General"
                }

                # 2b. 拷贝 RegisterConfig 内部内容
                if ($null -ne $src.RegisterConfig) {
                    if ($blockText.IndexOf('<RegisterConfig>') -ge 0) {
                        $blockText = Set-InnerFirst $blockText 'RegisterConfig' $src.RegisterConfig
                        Write-Host "[INFO] [$name] replaced RegisterConfig"
                    }
                    else {
                        # DST 该 Configuration 无 RegisterConfig，插入到 </Option> 之前
                        $insert = "`r`n`t`t`t`t<RegisterConfig>" + $src.RegisterConfig + "</RegisterConfig>"
                        $optIdx = $blockText.IndexOf('</Option>')
                        if ($optIdx -ge 0) {
                            # 回退到 </Option> 前的换行/缩进位置
                            $cut = $optIdx
                            while ($cut -gt 0 -and ($blockText[$cut - 1] -eq "`t" -or $blockText[$cut - 1] -eq ' ' -or $blockText[$cut - 1] -eq "`r" -or $blockText[$cut - 1] -eq "`n")) {
                                $cut--
                            }
                            $blockText = $blockText.Substring(0, $cut) + $insert + $blockText.Substring($cut)
                            Write-Host "[INFO] [$name] inserted RegisterConfig"
                        }
                    }
                }
            }

            [void]$sb.Append($blockText)
            $i = $b.EndPos
        }
        else {
            [void]$sb.Append($dstRaw[$i])
            $i++
        }
    }
    return $sb.ToString()
}

# --------------------------------------------------------
# 辅助函数：从文本中提取 Name="Linker Script" 的 Folder 块
# --------------------------------------------------------
function Get-LinkerScriptFolder {
    param([string]$text)
    $i = 0
    $len = $text.Length
    while ($i -lt $len) {
        if (($i + 8 -le $len) -and ($text.Substring($i, 8) -eq '<Folder>')) {
            $block = Get-Block $text $i 'Folder'
            if ($null -ne $block) {
                if ($block.Content -match 'Name="Linker Script"') {
                    return $block
                }
                $i = $block.EndPos
            }
            else { $i++ }
        }
        else { $i++ }
    }
    return $null
}

# --------------------------------------------------------
# 核心函数：把 srcInner 里所有 Folder/File 搬到结果
# 但遇到 Name="Linker Script" 的 Folder 时跳过（用 dst 的替代）
# --------------------------------------------------------
function Filter-ExcludeLinker {
    param([string]$text)
    $sb  = New-Object System.Text.StringBuilder
    $i   = 0
    $len = $text.Length
    while ($i -lt $len) {
        if (($i + 8 -le $len) -and ($text.Substring($i, 8) -eq '<Folder>')) {
            $block = Get-Block $text $i 'Folder'
            if ($null -eq $block) { [void]$sb.Append($text[$i]); $i++; continue }

            if ($block.Content -match 'Name="Linker Script"') {
                # 跳过 SRC 的 Linker Script Folder
                Write-Host "[INFO] Skip Linker Script Folder from SRC"
            }
            else {
                [void]$sb.Append($block.Content)
            }
            $i = $block.EndPos
        }
        elseif (($i + 6 -le $len) -and ($text.Substring($i, 6) -eq '<File>')) {
            $block = Get-Block $text $i 'File'
            if ($null -eq $block) { [void]$sb.Append($text[$i]); $i++; continue }
            [void]$sb.Append($block.Content)
            $i = $block.EndPos
        }
        else {
            [void]$sb.Append($text[$i])
            $i++
        }
    }
    return $sb.ToString()
}

# --------------------------------------------------------
# 1. 读取 SRC，提取 FilesTab 内部
# --------------------------------------------------------
$srcRaw = Get-Content -Raw -Encoding UTF8 $srcProj
Write-Host "[INFO] Read SRC: $($srcRaw.Length) chars"

$srcMatch = [regex]::Match($srcRaw, '(?s)<FilesTab>(.*?)</FilesTab>')
if (-not $srcMatch.Success) {
    Write-Host "[ERROR] No <FilesTab> in source proj"
    exit 1
}
$srcFilesInner = $srcMatch.Groups[1].Value

# 过滤掉 SRC 里的 Linker Script Folder
$srcFilesFiltered = Filter-ExcludeLinker $srcFilesInner

# --------------------------------------------------------
# 2. 读取 DST，切出 FilesTab 前/内/后
# --------------------------------------------------------
$dstRaw = Get-Content -Raw -Encoding UTF8 $dstProj
Write-Host "[INFO] Read DST: $($dstRaw.Length) chars"

$dstMatch = [regex]::Match($dstRaw, '(?s)(.*?)<FilesTab>(.*?)</FilesTab>(.*)')
if (-not $dstMatch.Success) {
    Write-Host "[ERROR] No <FilesTab> in target proj"
    exit 1
}
$dstBefore     = $dstMatch.Groups[1].Value
$dstFilesInner = $dstMatch.Groups[2].Value
$dstAfter      = $dstMatch.Groups[3].Value

# 从 DST 的 FilesTab 里提取 Linker Script Folder（保留原样）
$linkerBlock = Get-LinkerScriptFolder $dstFilesInner
if ($null -ne $linkerBlock) {
    $linkerContent = $linkerBlock.Content
    Write-Host "[INFO] Found Linker Script Folder in DST, will keep it"
}
else {
    $linkerContent = ''
    Write-Host "[WARN] No Linker Script Folder found in DST"
}

# --------------------------------------------------------
# 3. 拼接新的 FilesTab
#    顺序：SRC过滤内容（不含Linker）+ DST的Linker Script Folder
# --------------------------------------------------------
$newFilesInner = $srcFilesFiltered + $linkerContent
$newFilesTab   = '<FilesTab>' + $newFilesInner + '</FilesTab>'

# --------------------------------------------------------
# 4. 拼接全文件 + 清理空行 + 写回
# --------------------------------------------------------
$dstNew = $dstBefore + $newFilesTab + $dstAfter

# 拷贝 SRC 的 RegisterConfig 与 Compiler->General 到 DST（按 Configuration Name 匹配）
$dstNew = Copy-ConfigSettings $srcRaw $dstNew

$dstNew = $dstNew -replace '(\r?\n)(\s*\r?\n)+', "`r`n"
Set-Content -Encoding UTF8 $dstProj $dstNew

Write-Host "[OK] Updated: $dstProj  ($($dstNew.Length) chars)"