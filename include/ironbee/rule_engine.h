/*****************************************************************************
 * Licensed to Qualys, Inc. (QUALYS) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * QUALYS licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef _IB_RULE_ENGINE_H_
#define _IB_RULE_ENGINE_H_

/**
 * @file
 * @brief IronBee - Rule engine definitions
 *
 * @author Nick LeRoy <nleroy@qualys.com>
 */

#include <ironbee/build.h>
#include <ironbee/types.h>
#include <ironbee/rule_defs.h>
#include <ironbee/operator.h>
#include <ironbee/action.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Rule flag update operations.
 */
typedef enum {
    FLAG_OP_SET,                    /**< Set the flags */
    FLAG_OP_OR,                     /**< Or in the specified flags */
    FLAG_OP_CLEAR,                  /**< Clear the specified flags */
} ib_rule_flagop_t;

/**
 * Rule action add operator.
 */
typedef enum {
    RULE_ACTION_TRUE,               /**< Add a True action */
    RULE_ACTION_FALSE,              /**< Add a False action */
} ib_rule_action_t;

/**
 * Field operator function type.
 *
 * @param[in] ib Ironbee engine.
 * @param[in] mp Memory pool to use.
 * @param[in] field The field to operate on.
 * @param[out] result The result of the operator 1=true 0=false.
 *
 * @returns IB_OK if successful.
 */
typedef ib_status_t (* ib_field_op_fn_t)(ib_engine_t *ib,
                                         ib_mpool_t *mp,
                                         ib_field_t *field,
                                         ib_field_t **result);

/**
 * Rule engine: Rule meta data
 */
typedef struct {
    const char            *id;            /**< Rule ID */
    const char            *msg;           /**< Rule message */
    ib_list_t             *tags;          /**< Rule tags */
    ib_rule_phase_t        phase;         /**< Rule execution phase */
    uint8_t                severity;      /**< Rule severity */
    uint8_t                confidence;    /**< Rule confidence */
} ib_rule_meta_t;

/**
 * Rule engine: Rule list
 */
typedef struct {
    ib_list_t             *rule_list;     /**< List of rules */
} ib_rulelist_t;

/**
 * Rule engine: Target fields
 */
typedef struct {
    const char            *field_name;    /**< The field name */
    ib_list_t             *field_ops;     /**< List of field operators */
} ib_rule_target_t;

/**
 * Rule engine: Rule
 *
 * The typedef of ib_rule_t is done in ironbee/rule_engine.h
 */
struct ib_rule_t {
    ib_rule_meta_t         meta;          /**< Rule meta data */
    ib_operator_inst_t    *opinst;        /**< Rule operator */
    ib_list_t             *target_fields; /**< List of target fields */
    ib_list_t             *true_actions;  /**< Actions if condition True */
    ib_list_t             *false_actions; /**< Actions if condition False */
    ib_rulelist_t         *parent_rlist;  /**< Parent rule list */
    ib_rule_t             *chained_rule;  /**< Next rule in the chain */
    ib_flags_t             flags;         /**< External, etc. */
};

/**
 * Rule engine: List of rules to execute during a phase
 */
typedef struct {
    ib_rule_phase_t        phase;         /**< Phase number */
    ib_rulelist_t          rules;         /**< Rules to execute in phase */
} ib_rule_phase_data_t;

/**
 * Rule engine: Set of rules for all phases
 */
typedef struct {
    ib_rule_phase_data_t  phases[IB_RULE_PHASE_COUNT];
} ib_ruleset_t;


/**
 * Rule engine parser data
 */
typedef struct {
    ib_rule_t         *previous;     /**< Previous rule parsed */
} ib_rule_parser_data_t;

/**
 * Rule engine data; typedef in ironbee_private.h
 */
struct ib_rule_engine_t {
    ib_ruleset_t          ruleset;     /**< Rules to exec */
    ib_rulelist_t         rule_list;   /**< All rules owned by this context */
    ib_rule_parser_data_t parser_data; /**< Rule parser specific data */
};

/**
 * Create a rule.
 *
 * Allocates a rule for the rule engine, initializes it.
 *
 * @param[in] ib IronBee engine
 * @param[in] ctx Current IronBee context
 * @param[out] prule Address which new rule is written
 *
 * @returns Status code
 */
ib_status_t DLL_PUBLIC ib_rule_create(ib_engine_t *ib,
                                      ib_context_t *ctx,
                                      ib_rule_t **prule);

/**
 * Set a rule's operator.
 *
 * @param[in] ib IronBee engine
 * @param[in,out] rule Rule to operate on
 * @param[in] opinst Operator instance
 *
 * @returns Status code
 */
ib_status_t DLL_PUBLIC ib_rule_set_operator(ib_engine_t *ib,
                                            ib_rule_t *rule,
                                            ib_operator_inst_t *opinst);

/**
 * Set a rule's ID.
 *
 * @param[in] ib IronBee engine
 * @param[in,out] rule Rule to operate on
 * @param[in] id Rule ID
 *
 * @returns Status code
 */
ib_status_t DLL_PUBLIC ib_rule_set_id(ib_engine_t *ib,
                                      ib_rule_t *rule,
                                      const char *id);

/**
 * Get a rule's ID string.
 *
 * @param[in] rule Rule to operate on
 *
 * @returns Status code
 */
const char DLL_PUBLIC *ib_rule_id(const ib_rule_t *rule);

/**
 * Update a rule's flags.
 *
 * @param[in] ib IronBee engine
 * @param[in,out] rule Rule to operate on
 * @param[in] op Flag operation
 * @param[in] flags Flags to operate on
 *
 * @returns Status code
 */
ib_status_t DLL_PUBLIC ib_rule_update_flags(ib_engine_t *ib,
                                            ib_rule_t *rule,
                                            ib_rule_flagop_t op,
                                            ib_flags_t flags);

/**
 * Get a rule's flags.
 *
 * @param[in] rule The rule
 *
 * @returns The rule's flags
 */
ib_flags_t DLL_PUBLIC ib_rule_flags(const ib_rule_t *rule);

/**
 * Add an target field to a rule.
 *
 * @param[in] ib IronBee engine
 * @param[in,out] rule Rule to operate on
 * @param[in] name target field name.
 *
 * @returns Status code
 */
ib_status_t DLL_PUBLIC ib_rule_add_target(ib_engine_t *ib,
                                          ib_rule_t *rule,
                                          const char *name);

/**
 * Add a modifier to a rule.
 *
 * @param[in] ib IronBee engine
 * @param[in,out] rule Rule to operate on
 * @param[in] str Modifier string
 *
 * @returns Status code
 */
ib_status_t DLL_PUBLIC ib_rule_add_modifier(ib_engine_t *ib,
                                            ib_rule_t *rule,
                                            const char *str);

/**
 * Add a modifier to a rule.
 *
 * @param[in] ib IronBee engine
 * @param[in,out] rule Rule to operate on
 * @param[in] action Action instance to add
 * @param[in] which Which action list to add to
 *
 * @returns Status code
 */
ib_status_t DLL_PUBLIC ib_rule_add_action(ib_engine_t *ib,
                                          ib_rule_t *rule,
                                          ib_action_inst_t *action,
                                          ib_rule_action_t which);

/**
 * Register a rule.
 *
 * Register a rule for the rule engine.
 *
 * @param[in] ib IronBee engine
 * @param[in,out] ctx Context in which to execute the rule
 * @param[in,out] rule Rule to register
 * @param[in] phase Phase number in which to execute the rule
 *
 * @returns Status code
 */
ib_status_t DLL_PUBLIC ib_rule_register(ib_engine_t *ib,
                                        ib_context_t *ctx,
                                        ib_rule_t *rule,
                                        ib_rule_phase_t phase);

/**
 * Get the memory pool to use for rule allocations.
 *
 * @param[in] ib IronBee engine
 *
 * @returns Pointer to the memory pool to use.
 */
ib_mpool_t DLL_PUBLIC *ib_rule_mpool(ib_engine_t *ib);

#ifdef __cplusplus
}
#endif

#endif /* _IB_RULE_ENGINE_H_ */
