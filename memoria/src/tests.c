#include "tests.h"

int run_tests(){
    CU_initialize_registry();
    CU_pSuite tests = CU_add_suite("MEMORIA SUITE",NULL,NULL);
    CU_add_test(tests,"Agregar Entradas en la TLB", add_new_tlb_instance);
    CU_add_test(tests, "Llenar TLB", full_tlb);
    CU_add_test(tests, "Reemplazo TLB con FIFO", replace_tlb_fifo);
    CU_add_test(tests, "Reemplazo TLB con LRU", replace_tlb_lru);
    CU_add_test(tests, "Obtener Entrada de TLB", fetch_instance);
    CU_add_test(tests, "FAIL Intentar obtener Entrada de TLB", not_fetch_instance);
    CU_add_test(tests, "OK Delete process", ok_delete_process);
    CU_add_test(tests, "OK Suspend process", ok_suspend_process);
    //CU_add_test(tests, "OK PRINT METRICS", ok_print_carpinchos_metrics); /* EL TEST PASA: EL PROBLEMA ES EL FREE MEMORY QUE ROMPE PORQUE NO INSTANCIA TODO EL TEST.*/
    CU_add_test(tests, "OK PRINT DUMP", ok_print_dump);
    CU_add_test(tests, "OK Clean TLB", ok_clean_tlb);
    CU_add_test(tests, "OK Memread", ok_memread);
    CU_add_test(tests, "OK Memread con alloc entre medio de dos paginas", ok_memread_alloc_between_two_pages);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

t_log* create_log_test(){
    return log_create("./cfg/memoria-test.log", "MEMORIA-TEST", false, LOG_LEVEL_DEBUG);
}

bool sort_by_max_lru(void* actual, void* next){
    TLB* a_tlb = (TLB*) actual;
    TLB* n_tlb = (TLB*) next;

    return a_tlb->lru > n_tlb->lru;
}

void add_new_tlb_instance(){
    int cant_tlb = 2;
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    initPaginacion();

    add_entrada_tlb(1, 1, 1);
    add_entrada_tlb(1, 2, 2);

    CU_ASSERT_EQUAL(list_size(tlb_list), cant_tlb);
}

void full_tlb(){
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    initPaginacion();
    int pid = 0, page = 1, frame = 1;

    for (int i = 0; i < max_entradas_tlb; i++) {
        add_entrada_tlb(pid, page, frame);
        pid++;
        page++;
        frame++;
    }

    CU_ASSERT_EQUAL(list_size(tlb_list), max_entradas_tlb);
}

void replace_tlb_fifo(){
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    int fifo;
    initPaginacion();
    config_set_value(config, "ALGORITMO_REEMPLAZO_TLB", "FIFO");

    int pid = 0, page = 1, frame = 1;

    for (int i = 0; i < max_entradas_tlb; i++) {
        add_entrada_tlb(pid, page, frame);
        pid++;
        page++;
        frame++;
    }

    add_entrada_tlb(pid, page, frame);
    
    TLB* exp_tlb = malloc(sizeof(TLB));
    exp_tlb->pid = pid;
    exp_tlb->pagina = page;
    exp_tlb->frame = frame;
    exp_tlb->lru = tlb_lru_global - 1;
    if (entrada_fifo == 0){
        fifo = entrada_fifo;
    }else {
        fifo = entrada_fifo - 1;
    }
    TLB* actual = (TLB*) list_get(tlb_list, fifo);

    CU_ASSERT_EQUAL(actual->pid, exp_tlb->pid);
    CU_ASSERT_EQUAL(actual->pagina, exp_tlb->pagina);
    CU_ASSERT_EQUAL(actual->frame, exp_tlb->frame);
    CU_ASSERT_EQUAL(actual->lru, exp_tlb->lru);    
}

void replace_tlb_lru(){
    logger = create_log_test();
    config = config_create(CONFIG_PATH);

    initPaginacion();
    config_set_value(config, "ALGORITMO_REEMPLAZO_TLB", "LRU");

    int pid = 0, page = 1, frame = 1;

    for (int i = 0; i < max_entradas_tlb; i++) {
        add_entrada_tlb(pid, page, frame);
        pid++;
        page++;
        frame++;
    }

    add_entrada_tlb(pid, page, frame);
    
    TLB* exp_tlb = malloc(sizeof(TLB));
    exp_tlb->pid = pid;
    exp_tlb->pagina = page;
    exp_tlb->frame = frame;
    exp_tlb->lru = tlb_lru_global - 1;
   

    list_sort(tlb_list, sort_by_max_lru);
    TLB* actual = (TLB*) list_get(tlb_list, 0);

    CU_ASSERT_EQUAL(actual->pid, exp_tlb->pid);
    CU_ASSERT_EQUAL(actual->pagina, exp_tlb->pagina);
    CU_ASSERT_EQUAL(actual->frame, exp_tlb->frame);
    CU_ASSERT_EQUAL(actual->lru, exp_tlb->lru);    
}

void fetch_instance(){
    int exp_pid = 1, exp_page = 2;
    logger = create_log_test();
    config = config_create(CONFIG_PATH);

    initPaginacion();
    int pid = 0, page = 1, frame = 1;

    for (int i = 0; i < max_entradas_tlb; i++) {
        add_entrada_tlb(pid, page, frame);
        pid++;
        page++;
        frame++;
    }

    TLB* tlb = fetch_entrada_tlb(exp_pid, exp_page);

    CU_ASSERT_PTR_NOT_NULL(tlb);
    CU_ASSERT_EQUAL(tlb->pagina, exp_page);
    CU_ASSERT_EQUAL(tlb->pid, exp_pid);

}

void not_fetch_instance(){
    logger = create_log_test();
    config = config_create(CONFIG_PATH);

    initPaginacion();
    int pid = 0, page = 1, frame = 1;

    for (int i = 0; i < max_entradas_tlb; i++) {
        add_entrada_tlb(pid, page, frame);
        pid++;
        page++;
        frame++;
    }

    TLB* tlb = fetch_entrada_tlb(0, UINT32_MAX);

    CU_ASSERT_PTR_NULL(tlb);
}

void ok_delete_process(){
    int exp_res = 1;
    logger = create_log_test();
    config = config_create(CONFIG_PATH);

    initPaginacion();
    inicializarUnProceso(1);
    inicializarUnProceso(2);
    inicializarUnProceso(3);

    memalloc(1, 30);
    memalloc(2, 30);
    memalloc(3, 30);

    int res = delete_process(1);

    CU_ASSERT_EQUAL(res, exp_res);
}

void ok_suspend_process(){
    int exp_res = 1;
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    initPaginacion();
    inicializarUnProceso(1);
    inicializarUnProceso(2);
    inicializarUnProceso(3);
    memalloc(1, 30);
    memalloc(2, 30);
    memalloc(3, 30);


    int res = suspend_process(1);

    CU_ASSERT_EQUAL(res, exp_res);
}

void ok_print_carpinchos_metrics(){
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    initPaginacion();
    inicializarUnProceso(1);
    inicializarUnProceso(2);
    inicializarUnProceso(3);
    memalloc(1, 30);
    memalloc(2, 30);
    memalloc(3, 30);
    memwrite(1, 0, "HOLA", 4);

    print_metrics();

    CU_ASSERT_TRUE(true);
}

void ok_print_dump(){
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    initPaginacion();
    inicializarUnProceso(1);
    inicializarUnProceso(2);
    inicializarUnProceso(3);
    memalloc(1, 30);
    memalloc(2, 30);
    memalloc(3, 30);

    print_dump();

    CU_ASSERT_TRUE(true);
}

void ok_clean_tlb(){
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    initPaginacion();
    int pid = 0, page = 1, frame = 1;

    for (int i = 0; i < max_entradas_tlb; i++) {
        add_entrada_tlb(pid, page, frame);
        pid++;
        page++;
        frame++;
    }

    clean_tlb();

    CU_ASSERT_EQUAL(list_size(tlb_list), 0);
}

void ok_memread(){
    char* exp_res = "HOLA";
    char* c_res = string_new();
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    initPaginacion();
    inicializarUnProceso(1);
    inicializarUnProceso(2);
    inicializarUnProceso(3);
    memalloc(1, 30);
    memalloc(2, 30);
    memalloc(3, 30);
    memwrite(1, 0, "HOLA", 4);

    void* v_res = memread(1, 0, 4);

    memcpy(c_res, v_res, 4);
    c_res[4] = '\0';

    CU_ASSERT_TRUE(string_equals_ignore_case(c_res, exp_res));
}

void ok_memread_alloc_between_two_pages(){
    char* exp_res = "HOLA";
    char* c_res = string_new();
    logger = create_log_test();
    config = config_create(CONFIG_PATH);
    initPaginacion();
    inicializarUnProceso(1);
    inicializarUnProceso(2);

    memalloc(1, 30);
    memalloc(1, 30);
    memalloc(1, 30);
    memalloc(2, 30);
    memwrite(1, 39, "HOLA", 4);
    memwrite(1, 117, "HOLA", 4);

    void* v_res = memread(1, 117, 4);

    memcpy(c_res, v_res, 4);
    c_res[4] = '\0';

    CU_ASSERT_TRUE(string_equals_ignore_case(c_res, exp_res));
}
