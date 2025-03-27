@echo off

color a

set SourceFiles=Afstudeerverslag\Afstudeerverslag LiteratuurOnderzoek\LiteratuurOnderzoek SystemRequirementDocument\TestkastSRD FunctioneelOntwerp\FunctioneelOntwerp TechnischOntwerp\TechnischOntwerp Handleiding\Handleiding
set DestFolder=Output

rem Bewaar de huidige directory om later naar terug te keren
set CurrentDir=%CD%

rem Eerst de bestanden bouwen (pdflatex uitvoeren)
for %%f in (%SourceFiles%) do (
    rem Ga naar de map waar de .tex file zich bevindt
    cd /d "%%~dpf"

    echo ############################################################################
    echo
    echo Eerst een keertje pdflatex
    echo
    echo ############################################################################
    pdflatex -synctex=1 -interaction=batchmode "%%~nxf".tex

    echo ############################################################################
    echo
    echo Dan even bibtex compilen
    echo
    echo ############################################################################
    bibtex "%%~nxf"

    echo ############################################################################
    echo
    echo Glossaries compilen e.g. gebruikte acroniemen
    echo
    echo ############################################################################
    makeglossaries "%%~nxf"

    echo ############################################################################
    echo
    echo Tot slot nog 2x pdflatex compilen
    echo
    echo ############################################################################
    pdflatex -synctex=1 -interaction=batchmode "%%~nxf".tex
    pdflatex -synctex=1 -interaction=batchmode "%%~nxf".tex

    rem Keer terug naar de oorspronkelijke directory
    cd /d "%CurrentDir%"
)

echo ###############################################################################
echo
echo De .pdf bestanden kopieren naar de output folder
echo
echo ###############################################################################
for %%f in (%SourceFiles%) do (
    echo %%f.pdf kopieren naar %DestFolder%
    copy "%%f.pdf" "%DestFolder%"
)

pause