. "$PSScriptRoot\generate_descriptions.ps1"
$res = Parse-Script 'bonus bAtkEle,Ele_Water; bonus2 bAddEff,Eff_Freeze,500;'
foreach ($item in $res) {
    Write-Host ($item.GetType().FullName + " | " + $item)
}
