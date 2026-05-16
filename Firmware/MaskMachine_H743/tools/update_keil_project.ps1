param(
    [string]$ProjectPath = (Join-Path $PSScriptRoot '..\MDK-ARM\MaskMachine_H743.uvprojx')
)

$ProjectPath = (Resolve-Path $ProjectPath).Path
$FirmwareRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

Push-Location $FirmwareRoot
try {
    [xml]$xml = Get-Content -Path $ProjectPath -Raw
    $target = $xml.Project.Targets.Target
    $groupsNode = $target.Groups

    $includeAdd = @(
        '../User/Runtime',
        '../User/BSP',
        '../User/common',
        '../User/Drivers',
        '../User/Fonts',
        '../User/Services',
        '../Middlewares/LVGL',
        '../Middlewares/LVGL/lvgl',
        '../Middlewares/LVGL/lvgl/src'
    )

    foreach ($xpath in @(
        '/Project/Targets/Target/TargetOption/TargetArmAds/Cads/VariousControls/IncludePath',
        '/Project/Targets/Target/TargetOption/TargetArmAds/Aads/VariousControls/IncludePath'
    )) {
        $node = $xml.SelectSingleNode($xpath)
        if ($null -eq $node) {
            continue
        }

        $parts = @()
        if ($node.InnerText) {
            $parts += $node.InnerText.Split(';') | Where-Object { $_ -ne '' }
        }
        foreach ($inc in $includeAdd) {
            if ($parts -notcontains $inc) {
                $parts += $inc
            }
        }

        $node.InnerText = ($parts -join ';')
    }

    $defineNode = $xml.SelectSingleNode('/Project/Targets/Target/TargetOption/TargetArmAds/Cads/VariousControls/Define')
    if ($null -ne $defineNode) {
        $defs = @()
        if ($defineNode.InnerText) {
            $defs += $defineNode.InnerText.Split(',') | Where-Object { $_ -ne '' }
        }
        if ($defs -notcontains 'LV_CONF_INCLUDE_SIMPLE') {
            $defs += 'LV_CONF_INCLUDE_SIMPLE'
        }
        $defineNode.InnerText = ($defs -join ',')
    }

    $legacyAppGroup = 'User/' + 'App'
    foreach ($name in @($legacyAppGroup, 'User/Runtime', 'User/BSP', 'User/common', 'User/Drivers', 'User/Fonts', 'User/Services', 'Middlewares/LVGL')) {
        foreach ($existing in @($groupsNode.Group | Where-Object { $_.GroupName -eq $name })) {
            [void]$groupsNode.RemoveChild($existing)
        }
    }

    function New-TextElement([xml]$doc, [string]$name, [string]$text) {
        $el = $doc.CreateElement($name)
        $el.InnerText = $text
        return $el
    }

    function Add-Group([xml]$doc, $groups, [string]$name, [string[]]$paths) {
        $group = $doc.CreateElement('Group')
        [void]$group.AppendChild((New-TextElement $doc 'GroupName' $name))
        $files = $doc.CreateElement('Files')

        foreach ($path in ($paths | Sort-Object)) {
            $file = $doc.CreateElement('File')
            [void]$file.AppendChild((New-TextElement $doc 'FileName' ([IO.Path]::GetFileName($path))))
            [void]$file.AppendChild((New-TextElement $doc 'FileType' '1'))
            [void]$file.AppendChild((New-TextElement $doc 'FilePath' $path))
            [void]$files.AppendChild($file)
        }

        [void]$group.AppendChild($files)
        [void]$groups.AppendChild($group)
    }

    function To-UvPath([string]$fullPath) {
        $rel = $fullPath.Substring($FirmwareRoot.Length + 1).Replace('\', '/')
        return '../' + $rel
    }

    foreach ($dir in @('Runtime', 'BSP', 'common', 'Drivers', 'Fonts', 'Services')) {
        if (-not (Test-Path ".\User\$dir")) {
            continue
        }
        $files = Get-ChildItem -Path ".\User\$dir" -Filter *.c -File | ForEach-Object { To-UvPath $_.FullName }
        Add-Group $xml $groupsNode "User/$dir" ([string[]]$files)
    }

    $driverGroup = $groupsNode.Group | Where-Object { $_.GroupName -eq 'Drivers/STM32H7xx_HAL_Driver' } | Select-Object -First 1
    if ($null -ne $driverGroup) {
        $driverFilesNode = $driverGroup.Files
        $requiredHalDrivers = @(
            '../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c',
            '../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c',
            '../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c',
            '../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c',
            '../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c'
        )

        $existingDriverPaths = @()
        if ($null -ne $driverFilesNode) {
            $existingDriverPaths = @($driverFilesNode.File | ForEach-Object { $_.FilePath })
        }

        foreach ($path in $requiredHalDrivers) {
            if ($existingDriverPaths -contains $path) {
                continue
            }

            $file = $xml.CreateElement('File')
            [void]$file.AppendChild((New-TextElement $xml 'FileName' ([IO.Path]::GetFileName($path))))
            [void]$file.AppendChild((New-TextElement $xml 'FileType' '1'))
            [void]$file.AppendChild((New-TextElement $xml 'FilePath' $path))
            [void]$driverFilesNode.AppendChild($file)
        }
    }

    $lvglDirs = @(
        '.\Middlewares\LVGL\lvgl\src\core',
        '.\Middlewares\LVGL\lvgl\src\draw',
        '.\Middlewares\LVGL\lvgl\src\draw\sw',
        '.\Middlewares\LVGL\lvgl\src\font',
        '.\Middlewares\LVGL\lvgl\src\hal',
        '.\Middlewares\LVGL\lvgl\src\misc',
        '.\Middlewares\LVGL\lvgl\src\widgets',
        '.\Middlewares\LVGL\lvgl\src\extra',
        '.\Middlewares\LVGL\lvgl\src\extra\layouts\flex',
        '.\Middlewares\LVGL\lvgl\src\extra\layouts\grid',
        '.\Middlewares\LVGL\lvgl\src\extra\themes\basic',
        '.\Middlewares\LVGL\lvgl\src\extra\themes\default',
        '.\Middlewares\LVGL\lvgl\src\extra\themes\mono',
        '.\Middlewares\LVGL\lvgl\src\extra\libs\qrcode'
    )

    $lvglFiles = @()
    foreach ($dir in $lvglDirs) {
        if (Test-Path $dir) {
            $lvglFiles += Get-ChildItem -Path $dir -Filter *.c -File | ForEach-Object { To-UvPath $_.FullName }
        }
    }
    if (Test-Path '.\Middlewares\LVGL\lvgl\src\extra\widgets') {
        $lvglFiles += Get-ChildItem -Path '.\Middlewares\LVGL\lvgl\src\extra\widgets' -Filter *.c -File -Recurse | ForEach-Object { To-UvPath $_.FullName }
    }
    Add-Group $xml $groupsNode 'Middlewares/LVGL' ([string[]]$lvglFiles)

    $settings = New-Object System.Xml.XmlWriterSettings
    $settings.Indent = $true
    $settings.Encoding = New-Object System.Text.UTF8Encoding($false)
    $writer = [System.Xml.XmlWriter]::Create($ProjectPath, $settings)
    $xml.Save($writer)
    $writer.Close()

    Write-Host ("Updated Keil project: {0} LVGL source files linked." -f $lvglFiles.Count)
}
finally {
    Pop-Location
}
