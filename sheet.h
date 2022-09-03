#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <map>
#include <set>

class Sheet : public SheetInterface
{
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    // ����� ���� ������ � ������������
    void InvalidateCell(const Position& pos);
    // ���������� ����������� ����� �������� ������� � ���������
    void AddDependentCell(const Position& main_cell, const Position& dependent_cell);
    // ������, ��������� �� ������ � pos
    const std::set<Position> GetDependentCells(const Position& pos);
    // �������� ������������ ������ � pos
    void DeleteDependencies(const Position& pos);

private:
    // ���� - ������, �������� - ��������� ������
    std::map<Position, std::set<Position>> cells_to_cells_;
    // ����
    std::vector<std::vector<std::unique_ptr<Cell>>> sheet_;

    int max_row_ = 0;
    int max_col_ = 0;
    bool area_is_valid_ = true;
};