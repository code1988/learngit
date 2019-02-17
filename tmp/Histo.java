import sun.jvm.hotspot.HotSpotAgent;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.oops.*;


import java.io.*;
import java.util.*;

import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.runtime.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.gc_interface.*;
import sun.jvm.hotspot.gc_implementation.g1.*;
import sun.jvm.hotspot.gc_implementation.parallelScavenge.*;
import sun.jvm.hotspot.utilities.*;

class ObjectHistogramElement1 {

  private Klass klass;
  private long  count; // Number of instances of klass          
  private long  size;  // Total size of all these instances     
  private long  oopSize;

  public ObjectHistogramElement1(Klass k) {
     klass = k;
     count = 0;
     size  = 0;
  }

  public void updateWith(Oop obj) {
    count = count + 1;
    size  = size  + obj.getObjectSize();
  }

  public int compare(ObjectHistogramElement1 other) {
    return (int) (other.size - size);
  }

  /** Klass for this ObjectHistogramElement1 */
  public Klass getKlass() {
    return klass;
  }

  /** Number of instances of klass */
  public long getCount() {
    return count;
  }

  /** Total size of all these instances */
  public long getSize() {
    return size;
  }

  private String getInternalName(Klass k) {
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    getKlass().printValueOn(new PrintStream(bos));
    // '*' is used to denote VM internal klasses.
    return "* " + bos.toString();
  }

  /** Human readable description **/
  public String getDescription() {
     Klass k = getKlass();
     if (k instanceof InstanceKlass) {
        return k.getName().asString().replace('/', '.');
     } else if (k instanceof ArrayKlass) {
       ArrayKlass ak = (ArrayKlass) k;
       if (k instanceof TypeArrayKlass) {
          TypeArrayKlass tak = (TypeArrayKlass) ak;
          return tak.getElementTypeName() + "[]";
       } else if (k instanceof ObjArrayKlass) {
          ObjArrayKlass oak = (ObjArrayKlass) ak;
          Klass bottom = oak.getBottomKlass();
          int dim = (int) oak.getDimension();
          StringBuffer buf = new StringBuffer();
          if (bottom instanceof TypeArrayKlass) {
            buf.append(((TypeArrayKlass) bottom).getElementTypeName());
          } else if (bottom instanceof InstanceKlass) {
            buf.append(bottom.getName().asString().replace('/', '.'));
          } else {
            throw new RuntimeException("should not reach here");
          }
          for (int i=0; i < dim; i++) {
            buf.append("[]");
          }
          return buf.toString();
       }
    }
    return getInternalName(k);
  }

  public static void titleOn(PrintStream tty) {
    tty.println("Object Histogram:");
    tty.println();
    tty.println("num " + "\t" + "  #instances" + "\t" + "#bytes" + "\t" + "Class description");
    tty.println("--------------------------------------------------------------------------");
  }

  public void printOn(PrintStream tty) {
    tty.print(count + "\t" + size + "\t");
    tty.print(getDescription());
    tty.println();
  }
}

// class ObjectHistogram1 extends ObjectHistogram {
 class ObjectHistogram1 implements HeapVisitor{
  public ObjectHistogram1() { map = new HashMap(); }
  private HashMap map;
  @Override
  public void prologue(long size) {
    System.out.println("total size = " + size);
  }

  @Override
  public boolean doObj(Oop obj) {
    Klass klass = obj.getKlass();       
    ObjectHistogramElement1 e; 
    if (!map.containsKey(klass)) {
        e = new ObjectHistogramElement1(klass);
        map.put(klass, e);
    } else {
        e = (ObjectHistogramElement1) map.get(klass);
    }
    e.updateWith(obj);
    return false;
  }

  public void epilogue() {}
  public List getElements() {
    List list = new ArrayList();
    list.addAll(map.values());
    Collections.sort(list, new Comparator() {
      public int compare(Object o1, Object o2) {
        return ((ObjectHistogramElement1) o1).compare((ObjectHistogramElement1) o2);
      }
    });
    return list;
  }

  public void print() { printOn(System.out); }

  public void printOn(PrintStream tty) {
    List list = getElements();
    ObjectHistogramElement1.titleOn(tty);
    Iterator iterator = list.listIterator();
    int num=0;
    int totalCount=0;
    //int totalSize=0;
    while (iterator.hasNext()) {
      ObjectHistogramElement1 el = (ObjectHistogramElement1) iterator.next();
      num++;
      totalCount+=el.getCount();
      //totalSize+=el.getSize();
      tty.print(num + ":" + "\t\t");
      el.printOn(tty);
    }
    //tty.println("Total : " + "\t" + totalCount + "\t" + totalSize);
    tty.println("Total : " + "\t" + totalCount);
  }
}


public class Histo {

    public static void main(String[] args) {
        String core = args[0];

        HotSpotAgent agent;
        agent = new HotSpotAgent();
        agent.attach("/home/code/Program/jdk1.8.0/bin/java", core);

        ObjectHeap heap = VM.getVM().getObjectHeap();
        //ObjectHeap1 heap = VM.getVM().getObjectHeap();
        //ObjectHistogram histogram = new ObjectHistogram();
        ObjectHistogram1 histogram = new ObjectHistogram1();
        long startTime = System.currentTimeMillis();
        heap.iterate(histogram);
        long endTime = System.currentTimeMillis();
        histogram.printOn(System.out);
        float secs = (float) (endTime - startTime) / 1000.0f;

        System.out.println("Heap traversal took " + secs + " seconds.");
    }

}
