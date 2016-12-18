#ifndef BOOL3STATE_H
#define BOOL3STATE_H


class Bool3State
{
    public:
        Bool3State();
        Bool3State(bool value);
        virtual ~Bool3State();

        enum State
        {
            UNDEF,
            TRUE,
            FALSE
        };

        void SetVal(bool val);
        void SetUndef();
        State GetVal() const;

    protected:
    private:
        State m_value;
};

#endif // BOOL3STATE_H
