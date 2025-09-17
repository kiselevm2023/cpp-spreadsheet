#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

struct PositionHasher {
    std::size_t operator()(Position p) const {
        auto h1 = std::hash<int>{}(p.row);
        auto h2 = std::hash<int>{}(p.col);
        return h1 ^ (h2 << 1);
    }
};

struct PositionComparator {
    bool operator()(const Position& lhs, const Position& rhs) const {
        return lhs == rhs;
    }
};

class Sheet : public SheetInterface {
public:
    using Table = std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher, PositionComparator>;

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    const Cell* GetCellData(Position pos) const;
    Cell* GetCellData(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
	Table table_;
};