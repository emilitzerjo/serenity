/*
 * Copyright (c) 2022-2023, Martin Falisse <mfalisse@outlook.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/CSS/Length.h>
#include <LibWeb/Layout/FormattingContext.h>

namespace Web::Layout {

enum class GridDimension {
    Row,
    Column
};

class OccupationGrid {
public:
    OccupationGrid(size_t column_count, size_t row_count);
    OccupationGrid();

    void maybe_add_column(size_t needed_number_of_columns);
    void maybe_add_row(size_t needed_number_of_rows);
    void set_occupied(size_t column_start, size_t column_end, size_t row_start, size_t row_end);
    void set_occupied(size_t column_index, size_t row_index);

    size_t column_count() { return m_occupation_grid[0].size(); }
    size_t row_count() { return m_occupation_grid.size(); }
    bool is_occupied(size_t column_index, size_t row_index);

private:
    Vector<Vector<bool>> m_occupation_grid;
};

class GridItem {
public:
    GridItem(Box const& box, int row, int row_span, int column, int column_span)
        : m_box(box)
        , m_row(row)
        , m_row_span(row_span)
        , m_column(column)
        , m_column_span(column_span)
    {
    }

    Box const& box() const { return m_box; }

    size_t raw_row_span() { return m_row_span; }
    size_t raw_column_span() { return m_column_span; }

    size_t gap_adjusted_row(Box const& grid_box) const;
    size_t gap_adjusted_column(Box const& grid_box) const;

private:
    JS::NonnullGCPtr<Box const> m_box;
    size_t m_row { 0 };
    size_t m_row_span { 1 };
    size_t m_column { 0 };
    size_t m_column_span { 1 };
};

class GridFormattingContext final : public FormattingContext {
public:
    explicit GridFormattingContext(LayoutState&, Box const& grid_container, FormattingContext* parent);
    ~GridFormattingContext();

    virtual void run(Box const&, LayoutMode, AvailableSpace const& available_space) override;
    virtual CSSPixels automatic_content_width() const override;
    virtual CSSPixels automatic_content_height() const override;

    Box const& grid_container() const { return context_box(); }

private:
    CSSPixels m_automatic_content_height { 0 };
    bool is_auto_positioned_row(CSS::GridTrackPlacement const&, CSS::GridTrackPlacement const&) const;
    bool is_auto_positioned_column(CSS::GridTrackPlacement const&, CSS::GridTrackPlacement const&) const;
    bool is_auto_positioned_track(CSS::GridTrackPlacement const&, CSS::GridTrackPlacement const&) const;

    struct TemporaryTrack {
        CSS::GridSize min_track_sizing_function;
        CSS::GridSize max_track_sizing_function;
        CSSPixels base_size { 0 };
        bool has_definite_base_size { false };
        CSSPixels growth_limit { 0 };
        CSSPixels space_to_distribute { 0 };
        CSSPixels planned_increase { 0 };
        bool is_gap { false };

        CSSPixels border_left { 0 };
        CSSPixels border_right { 0 };
        CSSPixels border_top { 0 };
        CSSPixels border_bottom { 0 };

        CSSPixels full_horizontal_size() const
        {
            return base_size + border_left + border_right;
        }

        CSSPixels full_vertical_size() const
        {
            return base_size + border_top + border_bottom;
        }

        TemporaryTrack(CSS::GridSize min_track_sizing_function, CSS::GridSize max_track_sizing_function)
            : min_track_sizing_function(min_track_sizing_function)
            , max_track_sizing_function(max_track_sizing_function)
        {
        }

        TemporaryTrack(CSS::GridSize track_sizing_function)
            : min_track_sizing_function(track_sizing_function)
            , max_track_sizing_function(track_sizing_function)
        {
        }

        TemporaryTrack(CSSPixels size, bool is_gap)
            : min_track_sizing_function(CSS::GridSize(CSS::Length::make_px(size)))
            , max_track_sizing_function(CSS::GridSize(CSS::Length::make_px(size)))
            , base_size(size)
            , is_gap(is_gap)
        {
        }

        TemporaryTrack()
            : min_track_sizing_function(CSS::GridSize::make_auto())
            , max_track_sizing_function(CSS::GridSize::make_auto())
        {
        }
    };

    struct GridArea {
        String name;
        size_t row_start { 0 };
        size_t row_end { 1 };
        size_t column_start { 0 };
        size_t column_end { 1 };
    };
    Vector<GridArea> m_valid_grid_areas;

    Vector<TemporaryTrack> m_grid_rows;
    Vector<TemporaryTrack> m_grid_columns;

    OccupationGrid m_occupation_grid;
    Vector<GridItem> m_grid_items;
    Vector<JS::NonnullGCPtr<Box const>> m_boxes_to_place;

    void determine_intrinsic_size_of_grid_container(AvailableSpace const& available_space);

    AvailableSize get_free_space(AvailableSize const& available_size, Vector<TemporaryTrack> const& tracks) const;

    int get_line_index_by_line_name(String const& line_name, CSS::GridTrackSizeList);
    CSSPixels resolve_definite_track_size(CSS::GridSize const&, AvailableSpace const&);
    size_t count_of_gap_tracks(Vector<TemporaryTrack> const& tracks) const;
    int count_of_repeated_auto_fill_or_fit_tracks(Vector<CSS::ExplicitGridTrack> const& track_list, AvailableSpace const&);
    int get_count_of_tracks(Vector<CSS::ExplicitGridTrack> const&, AvailableSpace const&);

    void build_valid_grid_areas();
    int find_valid_grid_area(String const& needle);

    void place_grid_items(AvailableSpace const& available_space);
    void place_item_with_row_and_column_position(Box const& child_box);
    void place_item_with_row_position(Box const& child_box);
    void place_item_with_column_position(Box const& child_box, int& auto_placement_cursor_x, int& auto_placement_cursor_y);
    void place_item_with_no_declared_position(Box const& child_box, int& auto_placement_cursor_x, int& auto_placement_cursor_y);

    void initialize_grid_tracks_from_definition(AvailableSpace const& available_space, Vector<CSS::ExplicitGridTrack> const& tracks_definition, Vector<TemporaryTrack>& tracks);
    void initialize_grid_tracks_for_columns_and_rows(AvailableSpace const&);
    void initialize_gap_tracks(AvailableSpace const&);

    void initialize_track_sizes(AvailableSize const& available_size, Vector<TemporaryTrack>& tracks);
    void resolve_intrinsic_track_sizes(GridDimension const dimension, AvailableSize const& available_space, Vector<TemporaryTrack>& tracks);
    void maximize_tracks(AvailableSize const& available_size, Vector<TemporaryTrack>& tracks);
    void expand_flexible_tracks(AvailableSize const& available_size, Vector<TemporaryTrack>& tracks);
    void stretch_auto_tracks(AvailableSize const& available_size, Vector<TemporaryTrack>& tracks);
    void run_track_sizing(GridDimension const dimension, AvailableSpace const& available_space, Vector<TemporaryTrack>& tracks);

    CSS::Size const& get_item_preferred_size(GridItem const&, GridDimension const) const;

    CSSPixels calculate_min_content_size(GridItem const&, GridDimension const) const;
    CSSPixels calculate_max_content_size(GridItem const&, GridDimension const) const;

    CSSPixels calculate_min_content_contribution(GridItem const&, GridDimension const) const;
    CSSPixels calculate_max_content_contribution(GridItem const&, GridDimension const) const;

    CSSPixels calculate_limited_min_content_contribution(GridItem const&, GridDimension const) const;
    CSSPixels calculate_limited_max_content_contribution(GridItem const&, GridDimension const) const;

    CSSPixels containing_block_size_for_item(GridItem const&, GridDimension const) const;
    AvailableSpace get_available_space_for_item(GridItem const&) const;

    CSS::Size const& get_item_minimum_size(GridItem const&, GridDimension const) const;
    CSSPixels content_size_suggestion(GridItem const&, GridDimension const) const;
    Optional<CSSPixels> specified_size_suggestion(GridItem const&, GridDimension const) const;
    CSSPixels content_based_minimum_size(GridItem const&, GridDimension const) const;
    CSSPixels automatic_minimum_size(GridItem const&, GridDimension const) const;
    CSSPixels calculate_minimum_contribution(GridItem const&, GridDimension const) const;
};

}
