/*
 * Defines the status registers of the 210x0 as bitfields.
 * This is most usefull for code which doesn't need to be fast.
 */

register union {
  struct {
    unsigned         :15;
    unsigned rnd32   :1;

    unsigned trunc   :1;
    unsigned         :1;
    unsigned alusat  :1;
    unsigned irpten  :1;

    unsigned nestm   :1;
    unsigned srrfl   :1;
    unsigned         :2;

    unsigned srrfh   :1;
    unsigned srd2l   :1;
    unsigned srd2h   :1;
    unsigned srd1l   :1;

    unsigned srd1h   :1;
    unsigned srcu    :1;
    unsigned br0     :1;
    unsigned         :1;
  } b;
  int r;
} mode1 asm("mode1");

register union {
  struct {
  
    unsigned       :12;

    unsigned cafrz :1;

    unsigned flg3_output  :1;
    unsigned flg2_output  :1;
    unsigned flg1_output  :1;
    unsigned flg0_output  :1;
    
    unsigned       :3;
    
    unsigned timen :1;
    
    unsigned cadis :1;
    
    unsigned irq3_edge :1;
    unsigned irq2_edge :1;
    unsigned irq1_edge :1;
    unsigned irq0_edge :1;
  } b;

  int r;

} mode2 asm("mode2");


register union {
  struct {
    unsigned cacc7 :1;
    unsigned cacc6 :1;
    unsigned cacc5 :1;
    unsigned cacc4 :1;
    unsigned cacc3 :1;
    unsigned cacc2 :1;
    unsigned cacc1 :1;
    unsigned cacc0 :1;

    unsigned       :1;

    unsigned flg3_value  :1;
    unsigned flg2_value  :1;
    unsigned flg1_value  :1;
    unsigned flg0_value  :1;

    unsigned btf   :1;
    
    unsigned       :2;
    
    unsigned ss    :1;
    unsigned sz    :1;
    unsigned sv    :1;
    unsigned af    :1;
    unsigned mi    :1;
    unsigned mu    :1;
    unsigned mv    :1;
    unsigned mn    :1;
    unsigned ai    :1;
    unsigned as    :1;
    unsigned ac    :1;
    unsigned an    :1;
    unsigned av    :1;
    unsigned az    :1;
  } b;

  int r;

} astat asm("astat");


struct interrupt_flags {
    unsigned sft7i : 1;
    unsigned sft6i : 1;
    unsigned sft5i : 1;
    unsigned sft4i : 1;
    unsigned sft3i : 1;
    unsigned sft2i : 1;
    unsigned sft1i : 1;
    unsigned sft0i : 1;
    unsigned       : 5;
    unsigned fltii : 1;
    unsigned fltui : 1;
    unsigned fltoi : 1;
    unsigned fixi  : 1;
    unsigned tmzli : 1;
    unsigned       : 1;
    unsigned cb15i : 1;
    unsigned cb7i  : 1;
    unsigned       : 2;
    unsigned irq0i : 1;
    unsigned irq1i : 1;
    unsigned irq2i : 1;
    unsigned irq3i : 1;
    unsigned tmzhi : 1;
    unsigned sovfi : 1;
    unsigned       : 1;
    unsigned rsti  : 1;
    unsigned       : 1;
};

register union {
  struct interrupt_flags b;
  int r;
} irptl asm("irptl");

register union {
  struct interrupt_flags b;
  int r;
} imask asm("imask");

register union {
  struct interrupt_flags b;
  int r;
} imaskp asm("imaskp");


#define abs(x) ({ typeof (x) t; asm("%0=abs %1;" : "=d" (t) : "d" (x)); t;})
