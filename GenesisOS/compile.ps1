# compile.ps1
$compileCmd = "i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra"
$linkerCmd  = "i686-elf-gcc -T linker.ld -o genesis_kernel.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "      Compilando o GenesisOS..." -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# 1. Criar Ramdisk TAR
Write-Host "Criando o Ramdisk TAR..." -ForegroundColor Yellow
tar -cf initrd.tar readme.txt
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERRO] Falha ao criar o ramdisk initrd.tar." -ForegroundColor Red
    Read-Host "Pressione qualquer tecla para continuar . . ."
    exit
}

# 2. Criar Imagem de Disco se nao existir
if (-not (Test-Path "disk.img")) {
    Write-Host "Criando disco virtual de 10MB..." -ForegroundColor Yellow
    qemu-img create -f raw disk.img 10M
}

# 3. Compilar Assembly
Write-Host "1/3 - Compilando o Assembly (boot.asm)..." -ForegroundColor Yellow
nasm -felf32 boot.asm -o boot.o
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERRO] Falha ao compilar boot.asm." -ForegroundColor Red
    Read-Host "Pressione qualquer tecla para continuar . . ."
    exit
}

# 4. Compilar C com loop de Auto-Heal (Auto-Correcao!)
$success = $false
$attempts = 0
$maxAttempts = 5

while (-not $success -and $attempts -lt $maxAttempts) {
    $attempts++
    Write-Host "2/3 - Compilando o C (kernel.c) - Tentativa $attempts..." -ForegroundColor Yellow
    
    # Executa a compilacao e captura os erros
    $logFile = "compiler_errors.log"
    cmd /c "$compileCmd 2> $logFile"
    
    if ($LASTEXITCODE -eq 0) {
        $success = $true
        Write-Host "Compilacao do C concluida com SUCESSO!" -ForegroundColor Green
        break
    }

    # Analisa o arquivo de logs procurando sugestoes "did you mean"
    $logContent = Get-Content $logFile -Raw
    
    # Captura linha, variavel incorreta e a sugestao sugerida pelo GCC
    if ($logContent -match "kernel\.c:(\d+):\d+: error: '([^']+)' undeclared.*did you mean '([^']+)'") {
        $lineNum = [int]$Matches[1]
        $oldWord = $Matches[2]
        $newWord = $Matches[3]

        Write-Host "[AUTO-HEAL] Erro encontrado no kernel.c (Linha $lineNum): '$oldWord' nao foi declarado." -ForegroundColor Red
        Write-Host "[AUTO-HEAL] Aplicando sugestao do GCC: Substituindo '$oldWord' por '$newWord'..." -ForegroundColor Green
        
        # Le o kernel.c e aplica a substituicao automatica na linha exata
        $lines = Get-Content "kernel.c"
        $lines[$lineNum - 1] = $lines[$lineNum - 1] -replace $oldWord, $newWord
        Set-Content "kernel.c" $lines
        
        Remove-Item $logFile -ErrorAction SilentlyContinue
    } else {
        Write-Host "[ERRO] Falha ao compilar kernel.c (Nenhuma auto-sugestao encontrada para corrigir)." -ForegroundColor Red
        Get-Content $logFile
        Remove-Item $logFile -ErrorAction SilentlyContinue
        Read-Host "Pressione qualquer tecla para continuar . . ."
        exit
    }
}

if (-not $success) {
    Write-Host "[ERRO] Limite de tentativas de auto-correcao atingido!" -ForegroundColor Red
    Read-Host "Pressione qualquer tecla para continuar . . ."
    exit
}

# 5. Linkar os arquivos finais
Write-Host "3/3 - Linkando arquivos para genesis_kernel.bin..." -ForegroundColor Yellow
cmd /c "$linkerCmd"
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERRO] Falha ao linkar genesis_kernel.bin." -ForegroundColor Red
    Read-Host "Pressione qualquer tecla para continuar . . ."
    exit
}

Write-Host "==========================================" -ForegroundColor Green
Write-Host "    Compilacao concluida com sucesso!" -ForegroundColor Green
Write-Host "    Iniciando o GenesisOS no QEMU..." -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green

qemu-system-i386 -kernel genesis_kernel.bin -initrd initrd.tar -drive file=disk.img,format=raw,index=0,media=disk