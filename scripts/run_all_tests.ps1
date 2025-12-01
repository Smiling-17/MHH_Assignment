# PowerShell script to run all test cases
# Usage: .\scripts\run_all_tests.ps1

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  PETRI NET SOLVER - RUN ALL TESTS" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Clear old results
$csvPath = "output\result.csv"
if (Test-Path $csvPath) {
    Remove-Item $csvPath
    Write-Host "Cleared old result.csv" -ForegroundColor Yellow
}

# Get all PNML files
$pnmlFiles = Get-ChildItem -Path "data" -Filter "*.pnml" | Sort-Object Name

Write-Host "Found $($pnmlFiles.Count) test files" -ForegroundColor Green
Write-Host ""

# Run each test
foreach ($file in $pnmlFiles) {
    Write-Host "Testing: $($file.Name)" -ForegroundColor White
    & ".\bin\petri_solver.exe" --input "data\$($file.Name)" --mode all
    Write-Host ""
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  ALL TESTS COMPLETED!" -ForegroundColor Green
Write-Host "  Results saved to: output\result.csv" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

# Show summary
Write-Host ""
Write-Host "Run 'python scripts\analyze_results.py' to generate charts"



