@echo off

set SourceFiles=Afstudeerverslag\Afstudeerverslag LiteratuurOnderzoek\LiteratuurOnderzoek SystemRequirementDocument\TestkastSRD FunctioneelOntwerp\FunctioneelOntwerp TechnischOntwerp\TechnischOntwerp Handleiding\Handleiding
set DestFolder=Output

rem Bewaar de huidige directory om later naar terug te keren
set CurrentDir=%CD%

rem Eerst de bestanden bouwen (pdflatex uitvoeren)
for %%f in (%SourceFiles%) do (
    rem Ga naar de map waar de .tex file zich bevindt
    cd /d "%%~dpf"

    echo Eerst een keertje pdflatex
    pdflatex -synctex=1 -interaction=batchmode "%%~nxf".tex

    echo Dan even bibtex compilen
    bibtex "%%~nxf"

    echo Glossaries compilen e.g. gebruikte acroniemen
    makeglossaries "%%~nxf"

    echo Tot slot nog 2x pdflatex compilen
    pdflatex -synctex=1 -interaction=batchmode "%%~nxf".tex
    pdflatex -synctex=1 -interaction=batchmode "%%~nxf".tex

    rem Keer terug naar de oorspronkelijke directory
    cd /d "%CurrentDir%"
)

rem Nu de gegenereerde PDF's kopiëren
for %%f in (%SourceFiles%) do (
    echo %%f.pdf kopieren naar %DestFolder%
    copy "%%f.pdf" "%DestFolder%"
)

pause