#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    const auto& it = table_.find(pos);
    
    if (it == table_.end()) {
        table_.emplace(pos, std::make_unique<Cell>(*this));
    }

    table_.at(pos)->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellData(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetCellData(pos);
}

const Cell* Sheet::GetCellData(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    const auto& it = table_.find(pos);
    if (it == table_.end()) {
        return nullptr;
    }

    return it->second.get();
}

Cell* Sheet::GetCellData(Position pos) {
    return const_cast<Cell*>(static_cast<const Sheet&>(*this).GetCellData(pos));
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    const auto& it = table_.find(pos);
    if (it != table_.end() && it->second != nullptr) {
        it->second->Clear();
        if (it->second->GetText().empty()) {
            it->second.reset();
        }
    }
}

Size Sheet::GetPrintableSize() const {
    int max_row = -1;
    int max_col = -1;
    Size size {max_row, max_col};

    for (const auto& [pos, cell] : table_) {
        if (cell != nullptr) {
            size.rows = std::max(size.rows, pos.row);
            size.cols = std::max(size.cols, pos.col);
        }
    }

    return {size.rows + 1, size.cols + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            Position pos = {row, col};
            const auto& it = table_.find(pos);
            if (it != table_.end() && it->second != nullptr) {
                std::visit(
                    [&](const auto& value) { 
                        output << value; 
                    },
                it->second->GetValue());
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            Position pos = {row, col};
            const auto& it = table_.find(pos);
            if (it != table_.end() && it->second != nullptr) {
                output << it->second->GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}