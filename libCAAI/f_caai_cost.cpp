/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：对消耗的原辅料进行金额计算。
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
double f_caai_getprice(const CString& mat_code, const CString& price_terms, const CString& dept_code, const CString& stats_period, CDbConnection * conn);
BM2_FUNCTION_EXPORT
int f_caai_cost(CString stats_period, CString dept_code, CString price_terms, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal unit_price;

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	try
	{
		sqlstr =// "select distinct mat_code,company_code,sub_Backlog_code"
			" select distinct case when length(mat_code)>19 then 'PL_'||sub_Backlog_code else mat_code end mat_code,company_code"
			" from tcaai02 "
			" where 1=1 "
			" AND  rule_type in ('1','2','4','5') "  //4,5代表能源
			" AND DEPT_CODE = DECODE(@dept_code, ' ',dept_code,@dept_code)"
			" AND STATS_PERIOD = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			unit_price = f_caai_getprice(cmd_inq.GetString(1), "YJ", dept_code, stats_period, conn);
			
			if (cmd_inq.GetString(1).SubstringNE(1, 3) == "PL_")
			{
				sqlstr = " update tcaai02"
					" set UNIT_PRICE = @unit_price"
					",amt = round(wt*@unit_price,2)"
					" where 1=1"
					" and sub_backlog_code =@sub_backlog_code"
					" and length(mat_code)>19"
					" and company_code = @company_code "
					" AND  rule_type in ('1','2') "
					" AND DEPT_CODE = DECODE(@dept_code, ' ',dept_code,@dept_code)"
					" AND STATS_PERIOD = @stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("company_code", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("sub_backlog_code", cmd_inq.GetString(1).SubstringNE(4,cmd_inq.GetString(1).GetLength()-3));
				cmd_inq_1.Parameters.Set("unit_price", unit_price);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
			else
			{

				sqlstr = " update tcaai02"
					" set UNIT_PRICE = @unit_price"
					",amt = round(wt*@unit_price,2)"
					" where mat_code = @mat_code"
					" and company_code = @company_code "
					" AND  rule_type in ('1','2') "
					" AND DEPT_CODE = DECODE(@dept_code, ' ',dept_code,@dept_code)"
					" AND STATS_PERIOD = @stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("company_code", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
				cmd_inq_1.Parameters.Set("unit_price", unit_price);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}
		}
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1, 1, "[%s]", s.sysmsg);
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}
