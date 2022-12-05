function calculateBetweenDates(begin,end) {
    var beginDate = new Date(begin);
    var endDate = new Date(end);
    var diff = endDate - beginDate;
    var days = diff / (1000 * 60 * 60 * 24);
    return days;
}