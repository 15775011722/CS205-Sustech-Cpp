import java.util.*;

public class Project2 {
    private static final int maxInteger = Short.MAX_VALUE + 1;
    private static final int minInteger = Short.MIN_VALUE;
    private static final long maxLong = Integer.MAX_VALUE + 1L;
    private static final long minLong = Integer.MIN_VALUE;
    private static final float maxFloat = 1.8e19f;
    private static final float minFloat = -1.8e19f;
    private static final double maxDouble = 1.3e+150;
    private static final double minDouble = -1.3e+150;

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        System.out.print("Please input the size of vector: ");
        int t = sc.nextInt();
        MyVector v1 = new MyVector(t);
        MyVector v2 = new MyVector(t);
        v1.setName("vector1");
        v2.setName("vector2");
        System.out.print("""
                Do you want to limit the element type of vector?
                Short(0), Integer(1), Long(2), Float(3), Double(4), No Limit(Any other number)
                :\s""");
        int limit = sc.nextInt();
        System.out.println("Vectors are generating...");
        long start = System.nanoTime();
        generatingRandomNumber(v1, t, limit);
        generatingRandomNumber(v2, t, limit);
        long end = System.nanoTime();
        double cost = (end - start) / 1000000000.0;
        System.out.println("The cost time of generating random number is "+cost+"s");
        printVertor(v1, 6);
        printVertor(v2, 6);
        start = System.nanoTime();
        double result = v1.dotProduct(v2);
        end = System.nanoTime();
        cost = (end - start) / 1000000.0;
        System.out.println("+++++++++++++++++++++++++++++++++++++++++++");
        System.out.println("The dot product is: "+result);
        System.out.println("The cost time of dot product is "+cost+"ms");
    }

    /**
     * input: name(1 or 2)对应着vector1/vector2; size是向量大小，limitSize是指打印在终端操作台上的元素个数
     * output:打印向量的元素(限定个数)，并打印向量大小
     *  Print some element of vector
     **/
    static void printVertor(MyVector v, int limitSize) {
        if (limitSize == 0) limitSize = 6;
        int size = v.size;
        int example = Math.min(size, limitSize);
        System.out.print("vector"+v.name+": [");
        int idx = 0;
        for (; idx < example;idx++) {
            System.out.print(v.getElement(idx)+",");
        }
        System.out.printf(" (size %d)]\n", size);
    }

    /**
     *  input：向量 v，循环次数 t，限定类型 limit
     *  output：void
     *  为向量v补充随机大小元素，补充类型为short、int、long、float、double
     *  每种类型出现机会相同，皆为0.2概率
     *  当limit=0~4时为限定类型，其它数值皆表示随机类型
     **/
    static void generatingRandomNumber(MyVector v, int t, int limit) {
        Random random = new Random();
        for (int i = 0;i < t;i++) {
            random.setSeed(System.currentTimeMillis());
            int type = (limit > -1 && limit < 5) ? limit : random.nextInt(5);
            switch (type) {
                case 0 -> v.add((short) (random.nextInt(minInteger, maxInteger)));
                case 1 -> v.add(random.nextInt(minInteger, maxInteger));
                case 2 -> v.add(random.nextLong(minLong, maxLong));
                case 3 -> v.add(random.nextFloat(minFloat, maxFloat));
                case 4 -> v.add(random.nextDouble(minDouble, maxDouble));
                default -> i--;
            }
        }
    }
}

class MyVector {
    /* 由于不知道哪个更快，我决定都试一试(short除外，short不如int) */
//    private short[] shorts;
    private int[] ints;
    private long[] longs;
    private float[] floats;
    private double[] doubles;

    private final static HashSet<String> limitType = new HashSet<>(Arrays.asList("short", "int", "long", "float", "double"));

    private final static HashMap<String, String> type2Type = new HashMap<>();
    private static void mapTable() {
        type2Type.put("java.lang.Short", "short");
        type2Type.put("java.lang.Integer", "int");
        type2Type.put("java.lang.Long", "long");
        type2Type.put("java.lang.Float", "float");
        type2Type.put("java.lang.Double", "double");
    }

    int size;

    int maxSize;

    /* int=3 < long=4 < float=5 < double=6 故通过类名可以区分type*/
    int elementType;

    String name;

    public void setName(String n) {this.name = n;}

    /* int=3 < long=4 < float=5 < double=6 故通过类名可以区分type*/
    /* 没有类型保护！要是插入奇怪的就寄了。 */
    <T> void add(T e) {
        // 满了就不装了
        if (size >= maxSize) {
            System.out.println("Warning!The vector is full!!");
            return;
        }
        String type = type2Type.get(e.getClass().getName());
        if (!limitType.contains(type)) {
            System.err.println(type);
            System.err.println("错误的数据类型！！");
            return;
        }
        // 如果碰到short就转成int
        if(type.equals("short"))
            type = "int";
        int typeVal = type.length();
        if (elementType < typeVal) {
            switch (elementType+typeVal) {
                case 7 -> {
                    longs = new long[maxSize];
                    longs = Arrays.stream(ints).mapToLong(i -> i).toArray();
                    // 错误的方法
                    // System.arraycopy(ints, 0, longs, 0, size);
                    ints = null;
                }
                case 8 -> {
                    floats = new float[maxSize];
                    for (int i = 0;i < this.size;i++)
                        floats[i] = ints[i];
                    // System.arraycopy(ints, 0, floats, 0, size);
                    ints = null;
                }
                case 9 -> {
                    if (elementType == 3) {
                        doubles = new double[maxSize];
                        doubles = Arrays.stream(ints).mapToDouble(i -> i).toArray();
                        // System.arraycopy(ints, 0, doubles, 0, size);
                        ints = null;
                    }
                    else {
                        floats = new float[maxSize];
                        for (int i = 0;i < this.size;i++)
                            floats[i] = longs[i];
                        // System.arraycopy(longs, 0, floats, 0, size);
                        longs = null;
                    }
                }
                case 10 -> {
                    doubles = new double[maxSize];
                    doubles = Arrays.stream(longs).mapToDouble(i -> i).toArray();
                    // System.arraycopy(longs, 0, doubles, 0, size);
                    longs = null;
                }
                case 11 -> {
                    doubles = new double[maxSize];
                    for (int i = 0;i < this.size;i++)
                        doubles[i] = floats[i];
                    // System.arraycopy(floats, 0, doubles, 0, size);
                    floats = null;
                }
                default -> {
                    System.err.printf("elementType=%d; typeVal=%d\n", elementType, typeVal);
                    System.err.println("未知的错误，出现在数组类型转换。可能是add了奇怪的数据类型");
                    return;
                }
            }
            elementType = typeVal;
        }
        switch (elementType) {
            case 3 -> ints[size++] = Integer.parseInt(e.toString());
            case 4 -> longs[size++] = Long.parseLong(e.toString());
            case 5 -> floats[size++] = Float.parseFloat(e.toString());
            case 6 -> doubles[size++] = Double.parseDouble(e.toString());
            default -> System.err.println("未知错误，出现在数组类型指针，超出[3,6]范围。");
        }
    }

    public double get(int index) {
        double val = Double.NaN;
        switch (this.elementType) {
            case 3 -> val = ints[index];
            case 4 -> val = longs[index];
            case 5 -> val = floats[index];
            case 6 -> val = doubles[index];
            default -> System.err.println("错误的elementType出现在get函数中。");
        }
        return val;
    }

    public String getElement(int index) {
        String val = "";
        switch (this.elementType) {
            case 3 -> val = Integer.toString(ints[index]);
            case 4 -> val = Long.toString(longs[index]);
            case 5 -> val = Float.toString(floats[index]);
            case 6 -> val = Double.toString(doubles[index]);
            default -> System.err.println("错误的elementType出现在getElement函数中。");
        }
        return val;
    }

    public MyVector(int size) {
        this.maxSize = size;
        ints = new int[maxSize];
        this.elementType = 3;
        this.size = 0;
        mapTable();
    }

    double dotProduct(MyVector v) {
        if (v.size != this.size) {
            System.err.println("向量大小不一样，不能算点积！！");
            return 0;
        }
        double result = 0;
        // 映射双方数据类型到 [6,21]中
        int xType = (this.elementType<<2)-v.elementType;
        switch (xType) {
            case 6 -> {
                for (int i = 0;i < size; i++)
                    result += this.ints[i] * v.doubles[i];
            }
            case 7 -> {
                for (int i = 0;i < size; i++)
                    result += this.ints[i] * v.floats[i];
            }
            case 8 -> {
                for (int i = 0;i < size; i++)
                    result += this.ints[i] * v.longs[i];
            }
            case 9 -> {
                for (int i = 0;i < size; i++)
                    result += this.ints[i] * v.ints[i];
            }
            case 10 -> {
                for (int i = 0;i < size; i++)
                    result += this.longs[i] * v.doubles[i];
            }
            case 11 -> {
                for (int i = 0;i < size; i++)
                    result += this.longs[i] * v.floats[i];
            }
            case 12 -> {
                for (int i = 0;i < size; i++)
                    result += this.longs[i] * v.longs[i];
            }
            case 13 -> {
                for (int i = 0;i < size; i++)
                    result += this.longs[i] * v.ints[i];
            }
            case 14 -> {
                for (int i = 0;i < size; i++)
                    result += this.floats[i] * v.doubles[i];
            }
            case 15 -> {
                for (int i = 0;i < size; i++)
                    result += this.floats[i] * v.floats[i];
            }
            case 16 -> {
                for (int i = 0;i < size; i++)
                    result += this.floats[i] * v.longs[i];
            }
            case 17 -> {
                for (int i = 0;i < size; i++)
                    result += this.floats[i] * v.ints[i];
            }
            case 18 -> {
                for (int i = 0;i < size; i++)
                    result += this.doubles[i] * v.doubles[i];
            }
            case 19 -> {
                for (int i = 0;i < size; i++)
                    result += this.doubles[i] * v.floats[i];
            }
            case 20 -> {
                for (int i = 0;i < size; i++)
                    result += this.doubles[i] * v.longs[i];
            }
            case 21 -> {
                for (int i = 0;i < size; i++)
                    result += this.doubles[i] * v.ints[i];
            }
            default -> System.err.println("未知错误，出现在点乘时数组类型指针，超出[3,6]范围。");
        }
        return result;
    }
}
