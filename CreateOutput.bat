@echo off

set SourceFiles=Afstudeerverslag\Afstudeerverslag.pdf LiteratuurOnderzoek\LiteratuurOnderzoek.pdf SystemRequirementDocument\TestkastSRD.pdf FunctioneelOntwerp\FunctioneelOntwerp.pdf

set DestFolder=Output

for %%f in (%SourceFiles%) do (
	copy "%%f" "%DestFolder%"
)

pause