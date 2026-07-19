@echo off
chcp 65001 >nul
setlocal

echo ============================================
echo         BANNY PROJECT ANALYZER
echo ============================================

if exist project_report.txt del project_report.txt

echo ============================================ >> project_report.txt
echo PROJECT TREE >> project_report.txt
echo ============================================ >> project_report.txt

tree /F >> project_report.txt

echo. >> project_report.txt
echo ============================================ >> project_report.txt
echo CMAKE FILE >> project_report.txt
echo ============================================ >> project_report.txt

type CMakeLists.txt >> project_report.txt

echo. >> project_report.txt
echo ============================================ >> project_report.txt
echo INDEX CLASSES >> project_report.txt
echo ============================================ >> project_report.txt

for %%A in (
RealtimeIndexer
TriangleIndex
IndexedTriangle
IndexedTriangleBuilder
DirtyQueue
IndexedMarketCache
SymbolRegistryIndex
MarketDataCache
WebSocketMarketDataProvider
MarketMonitor
ArbitrageEngine
Bot
) do (
    echo. >> project_report.txt
    echo ############################ >> project_report.txt
    echo %%A >> project_report.txt
    echo ############################ >> project_report.txt
    findstr /S /N /I "%%A" src\*.cpp src\*.h >> project_report.txt
)

echo. >> project_report.txt
echo ============================================ >> project_report.txt
echo HEADER FILES >> project_report.txt
echo ============================================ >> project_report.txt

for /R src %%F in (*.h) do (
    echo. >> project_report.txt
    echo ============================================ >> project_report.txt
    echo %%F >> project_report.txt
    echo ============================================ >> project_report.txt
    type "%%F" >> project_report.txt
)

echo. >> project_report.txt
echo ============================================ >> project_report.txt
echo CPP FILES >> project_report.txt
echo ============================================ >> project_report.txt

for /R src %%F in (*.cpp) do (
    echo. >> project_report.txt
    echo ============================================ >> project_report.txt
    echo %%F >> project_report.txt
    echo ============================================ >> project_report.txt
    type "%%F" >> project_report.txt
)

echo.
echo ============================================
echo DONE
echo project_report.txt CREATED
echo ============================================

pause