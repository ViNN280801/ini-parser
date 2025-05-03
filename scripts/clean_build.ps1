Remove-Item -Recurse -Force -Verbose build/*
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build directory successfully cleaned"
}
else {
    Write-Host "Failed to clean build directory"
}
