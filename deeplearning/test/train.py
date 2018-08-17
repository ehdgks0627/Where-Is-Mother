import json
import subprocess
import copy
import tensorflow as tf

OK = [1, 0]
INFECTED = [0, 1]

input_size = 20
# output_size = len(input_dirs)
output_size = 2
learning_rate = 0.005
epoch_size = 400
test_rate = 0.7
dropout_rate = 0.2

x_datas = []
y_datas = []
x_test_datas = []
y_test_datas = []
x_h_datas = []


def load_data(data_size=10000):
    raw_data = subprocess.check_output(["python3", "data.py", "--router=" + str(input_size), str(data_size)]).decode()
    y = json.loads(raw_data.split("\n")[0])
    x = json.loads(raw_data.split("\n")[1])
    return x, y


tf.set_random_seed(1004)

for MAX_LAYER in [4, 5, 6]:
    Ls = []
    x, y = load_data()
    buck = int(len(x) * test_rate)

    x_datas += copy.deepcopy(x[:buck])
    y_datas += copy.deepcopy(y[:buck])
    x_test_datas += copy.deepcopy(x[buck:])
    y_test_datas += copy.deepcopy(y[buck:])

    for net_size in range(50, 150, 10):
        tf.reset_default_graph()
        X = tf.placeholder(tf.float32, [None, input_size])
        Y = tf.placeholder(tf.float32, [None, output_size])

        keep_prob = tf.placeholder(tf.float32)
        for layer_index in range(1, MAX_LAYER):
            if layer_index == 1:
                W = tf.get_variable("W%d" % (layer_index), shape=[input_size, net_size],
                                    initializer=tf.contrib.layers.xavier_initializer())
                b = tf.Variable(tf.random_normal([net_size]))
            elif layer_index == MAX_LAYER - 1:
                W = tf.get_variable("W%d" % (layer_index), shape=[net_size, output_size],
                                    initializer=tf.contrib.layers.xavier_initializer())
                b = tf.Variable(tf.random_normal([output_size]))
            else:
                W = tf.get_variable("W%d" % (layer_index), shape=[net_size, net_size],
                                    initializer=tf.contrib.layers.xavier_initializer())
                b = tf.Variable(tf.random_normal([net_size]))

            if layer_index == 1:
                L = tf.nn.relu(tf.matmul(X, W) + b)
            elif layer_index == MAX_LAYER - 1:
                hypothesis = tf.matmul(Ls[-1], W) + b
            else:
                L = tf.nn.relu(tf.matmul(Ls[-1], W) + b)
                L = tf.nn.dropout(Ls[-1], keep_prob=keep_prob)
            Ls.append(L)

        cost = tf.reduce_sum(tf.pow(hypothesis - Y, 2)) / len(x_test_datas) / 1000

        optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate).minimize(cost)

        # correct_prediction = tf.equal(tf.argmax(hypothesis, 1), tf.argmax(Y, 1))
        accuracy = tf.reduce_mean(cost)

        cost_summary = tf.summary.scalar('cost', cost)
        accuracy_summary = tf.summary.scalar('accuracy', accuracy)
        merged = tf.summary.merge_all()

        for dropout_rate in [0.7]:
            sess = tf.Session()
            sess.run(tf.global_variables_initializer())

            writer = tf.summary.FileWriter(
                './log/layer(%d)-input(%d)-dropout(%s)' % (layer_index, input_size, dropout_rate), sess.graph)

            for epoch in range(epoch_size):
                feed_dict = {X: x_datas, Y: y_datas, keep_prob: dropout_rate}
                c, _, h, _y = sess.run([cost, optimizer, hypothesis, Y], feed_dict=feed_dict)
                if epoch % 10 == 0:
                    print('Epoch:', '%04d' % (epoch), 'cost =', '{:.9f}'.format(c))
                    print('Accuracy:',
                          1 / sess.run(accuracy, feed_dict={X: x_test_datas, Y: y_test_datas, keep_prob: 1}))
                    result = sess.run(merged, feed_dict={X: x_test_datas, Y: y_test_datas, keep_prob: 1})
                    writer.add_summary(result, epoch)

            print('Learning Finished!')

            # print('Prediction:',
            #      sess.run(correct_prediction, feed_dict={X: x_test_datas, Y: y_test_datas, keep_prob: 1}))
            print('Accuracy:', sess.run([accuracy], feed_dict={X: x_test_datas, Y: y_test_datas, keep_prob: 1}))

    '''
    h = sess.run(hypothesis, feed_dict={X: x_h_datas})
    import pickle
    f = open("h_data", "wb")
    pickle.dump(h, f)
    f.close()'''
