/*
 * Shutdown.h
 *
 *  Created on: Jul 17, 2017
 *      Author: root
 */

#ifndef UTILS_SHUTDOWN_H_
#define UTILS_SHUTDOWN_H_

class Shutdown {
public:
    Shutdown()
        : is_signal_received_ (false),
        signalService_(),
        signals_(signalService_, SIGINT, SIGTERM, SIGQUIT)
    {
        //std::cout<<"constructor"<<std::endl;
    }
    ~Shutdown()
    {
        signals_.cancel();
        signalService_.stop();
        signalThread_.join();
    }

    void init() {
        signals_.async_wait(boost::bind(&Shutdown::handleStop, this, _1, _2));
        signalThread_ = boost::thread(boost::bind(&boost::asio::io_service::run, &signalService_));
    }

    bool isSignalReceived() const {
        return is_signal_received_;
    }

private:
    std::atomic<bool> is_signal_received_;
    boost::asio::io_service signalService_;
    boost::thread signalThread_;
    boost::asio::signal_set signals_;

    void handleStop(const boost::system::error_code& error, int signal_number) {
        is_signal_received_ = true;
        myHandleStop(error, signal_number);
    }

    virtual void myHandleStop(const boost::system::error_code& error, int signal_number) {
    }
};




#endif /* UTILS_SHUTDOWN_H_ */
