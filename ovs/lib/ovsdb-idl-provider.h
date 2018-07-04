/* Copyright (c) 2009, 2010, 2011, 2012, 2016 Nicira, Inc.
 * Copyright (C) 2016 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OVSDB_IDL_PROVIDER_H
#define OVSDB_IDL_PROVIDER_H 1

#include "openvswitch/hmap.h"
#include "openvswitch/list.h"
#include "ovsdb-idl.h"
#include "ovsdb-map-op.h"
#include "ovsdb-set-op.h"
#include "ovsdb-types.h"
#include "openvswitch/shash.h"
#include "uuid.h"

/* A local copy of a row in an OVSDB table, replicated from an OVSDB server.
 * This structure is used as a header for a larger structure that translates
 * the "struct ovsdb_datum"s into easier-to-use forms, via the ->parse() and
 * ->unparse functions in struct ovsdb_idl_column.  (Those functions are
 * generated automatically via ovsdb-idlc.)
 * 用于描述ovsdb数据库table中的一个row
 *
 * When no transaction is in progress:
 *
 *     - 'old' points to the data committed to the database and currently
 *       in the row.
 *
 *     - 'new == old'.
 *
 * When a transaction is in progress, the situation is a little different.  For
 * a row inserted in the transaction, 'old' is NULL and 'new' points to the
 * row's initial contents.  Otherwise:
 *
 *     - 'old' points to the data committed to the database and currently in
 *       the row.  (This is the same as when no transaction is in progress.)
 *
 *     - If the transaction does not modify the row, 'new == old'.
 *
 *     - If the transaction modifies the row, 'new' points to the modified
 *       data.
 *
 *     - If the transaction deletes the row, 'new' is NULL.
 *
 * Thus:
 *
 *     - 'old' always points to committed data, except that it is NULL if the
 *       row is inserted within the current transaction.
 *
 *     - 'new' always points to the newest, possibly uncommitted version of the
 *       row's data, except that it is NULL if the row is deleted within the
 *       current transaction.
 */
struct ovsdb_idl_row {
    struct hmap_node hmap_node; /* In struct ovsdb_idl_table's 'rows'.  该row关联的hash节点模块 */
    struct uuid uuid;           /* Row "_uuid" field. */
    struct ovs_list src_arcs;   /* Forward arcs (ovsdb_idl_arc.src_node). */
    struct ovs_list dst_arcs;   /* Backward arcs (ovsdb_idl_arc.dst_node). */
    struct ovsdb_idl_table *table; /* Containing table. */
    struct ovsdb_datum *old;    /* Committed data (null if orphaned). */

    /* Transactional data. */
    struct ovsdb_datum *new;    /* Modified data (null to delete row). */
    unsigned long int *prereqs; /* Bitmap of columns to verify in "old". */
    unsigned long int *written; /* Bitmap of columns from "new" to write. */
    struct hmap_node txn_node;  /* Node in ovsdb_idl_txn's list. */
    unsigned long int *map_op_written; /* Bitmap of columns pending map ops. */
    struct map_op_list **map_op_lists; /* Per-column map operations. */
    unsigned long int *set_op_written; /* Bitmap of columns pending set ops. */
    struct set_op_list **set_op_lists; /* Per-column set operations. */

    /* Tracking data */
    unsigned int change_seqno[OVSDB_IDL_CHANGE_MAX];
    struct ovs_list track_node; /* Rows modified/added/deleted by IDL */
    unsigned long int *updated; /* Bitmap of columns updated by IDL */
};

// 用于描述ovsdb数据库table中的一个column
struct ovsdb_idl_column {
    char *name;     // column名,作为键值索引用
    struct ovsdb_type type;
    bool mutable;
    void (*parse)(struct ovsdb_idl_row *, const struct ovsdb_datum *);
    void (*unparse)(struct ovsdb_idl_row *);
};

// ovsdb数据库中一类table的抽象
struct ovsdb_idl_table_class {
    char *name;     // 类名,作为键值索引用
    bool is_root;   // 标识这类table是否为根表
    const struct ovsdb_idl_column *columns; // 指向一const数组,记录了该类table包含的columns
    size_t n_columns;                       // 数组中的column数量,显然也是个固定值
    size_t allocation_size;
    void (*row_init)(struct ovsdb_idl_row *);
};

// 用于描述ovsdb数据库中一个table
struct ovsdb_idl_table {
    const struct ovsdb_idl_table_class *class;  // 指向该table所属的类
    unsigned char *modes;    /* OVSDB_IDL_* bitmasks, indexed by column.  指向一个数组,数组长度跟这类table包含的column数量有关,
                                每个元素依次记录了对应column的mode */
    bool need_table;         /* Monitor table even if no columns are selected
                              * for replication. */
    struct shash columns;    /* Contains "const struct ovsdb_idl_column *"s. */
    struct hmap rows;        /* Contains "struct ovsdb_idl_row"s. */
    struct ovsdb_idl *idl;   /* Containing idl.  指向该table所属的ovsdb操作句柄 */
    unsigned int change_seqno[OVSDB_IDL_CHANGE_MAX];
    struct shash indexes;    /* Contains "struct ovsdb_idl_index"s */
    struct ovs_list track_list; /* Tracked rows (ovsdb_idl_row.track_node). */
    struct ovsdb_idl_condition condition;
    bool cond_changed;
};

// 用于描述一个完整的ovsdb数据库
struct ovsdb_idl_class {
    const char *database;       /* <db-name> for this database.  数据库名 */
    const struct ovsdb_idl_table_class *tables;     // 这张列表的每个条目就是一类table的集合，整张表记录了数据库的所有数据
    size_t n_tables;                                // 该列表中的条目数量
};

/*
 * Structure containing the per-column configuration of the index.
 */
struct ovsdb_idl_index_column {
    const struct ovsdb_idl_column *column; /* Column used for index key. */
    column_comparator *comparer; /* Column comparison function. */
    int sorting_order; /* Sorting order (ascending or descending). */
};

/*
 * Defines a IDL compound index
 */
struct ovsdb_idl_index {
    struct skiplist *skiplist;    /* Skiplist with pointers to rows. */
    struct ovsdb_idl_index_column *columns; /* Columns configuration */
    size_t n_columns;             /* Number of columns in index. */
    size_t alloc_columns;         /* Size allocated memory for columns,
                                     comparers and sorting order. */
    bool ins_del;                 /* True if a row in the index is being
                                     inserted or deleted; if true, the
                                     search key is augmented with the
                                     UUID and address in order to discriminate
                                     between entries with identical keys. */
    const struct ovsdb_idl_table *table; /* Table that owns this index */
    const char *index_name;       /* The name of this index. */
};

struct ovsdb_idl_row *ovsdb_idl_get_row_arc(
    struct ovsdb_idl_row *src,
    const struct ovsdb_idl_table_class *dst_table,
    const struct uuid *dst_uuid);

void ovsdb_idl_txn_verify(const struct ovsdb_idl_row *,
                          const struct ovsdb_idl_column *);

struct ovsdb_idl_txn *ovsdb_idl_txn_get(const struct ovsdb_idl_row *);

#endif /* ovsdb-idl-provider.h */
